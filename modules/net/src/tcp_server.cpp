#include "moth/net/tcp_server.h"

#include "moth/net/frame.h"

#include <moth/core/log.h>

#include <asio.hpp>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

namespace moth::net {
    using json = nlohmann::json;

    namespace {
        // Per-connection state machine. Reads a 4-byte length, then a body of
        // that length, repeats. Writes are queued in an outbound deque so
        // overlapping async_writes cannot stomp each other.
        struct Connection : std::enable_shared_from_this<Connection> {
            asio::ip::tcp::socket socket;
            std::array<unsigned char, kLengthPrefixBytes> lengthBuf{};
            std::vector<char> bodyBuf;
            std::deque<std::string> outbox;
            bool writing = false;
            TcpServer::ConnectionId id = TcpServer::kInvalidConnection;

            explicit Connection(asio::io_context& io) : socket(io) {}
        };
    }

    struct TcpServer::Impl {
        asio::io_context io;
        std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
        std::vector<std::shared_ptr<Connection>> connections;

        std::unordered_map<std::string, MessageHandler> handlers;
        ConnectionHandler connectedHandler;
        ConnectionHandler disconnectedHandler;

        ConnectionId nextId = 1;
        std::uint32_t seq = 0;

        std::shared_ptr<Connection> FindConnection(ConnectionId id) const {
            for (auto const& conn : connections) {
                if (conn->id == id) {
                    return conn;
                }
            }
            return nullptr;
        }

        json BuildPayload(std::string_view type, json const& body) {
            json payload = body.is_object() ? body : json::object();
            if (!body.is_object() && !body.is_null()) {
                payload["body"] = body;
            }
            payload["type"] = type;
            payload["seq"] = seq++;
            return payload;
        }

        void StartAccept();
        void HandleAccept(std::shared_ptr<Connection> const& conn, std::error_code ec);

        void StartReadLength(std::shared_ptr<Connection> const& conn);
        void StartReadBody(std::shared_ptr<Connection> const& conn, std::uint32_t length);
        void DispatchPayload(std::shared_ptr<Connection> const& conn);

        void SendFramed(std::shared_ptr<Connection> const& conn, json const& payload);
        void DoWriteNext(std::shared_ptr<Connection> const& conn);

        void DropConnection(std::shared_ptr<Connection> const& conn);
        void DropAll();
    };

    TcpServer::TcpServer() : m_impl(std::make_unique<Impl>()) {}

    TcpServer::~TcpServer() {
        Close();
    }

    void TcpServer::SetHandler(std::string_view type, MessageHandler handler) {
        if (handler) {
            m_impl->handlers[std::string(type)] = std::move(handler);
        } else {
            m_impl->handlers.erase(std::string(type));
        }
    }

    void TcpServer::SetConnectedHandler(ConnectionHandler handler) {
        m_impl->connectedHandler = std::move(handler);
    }

    void TcpServer::SetDisconnectedHandler(ConnectionHandler handler) {
        m_impl->disconnectedHandler = std::move(handler);
    }

    bool TcpServer::Listen(std::uint16_t port) {
        if (m_impl->acceptor) {
            return true;
        }
        try {
            asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
            m_impl->acceptor = std::make_unique<asio::ip::tcp::acceptor>(m_impl->io, endpoint);
            m_impl->StartAccept();
            moth::core::log::info("net: listening on port {}", port);
            return true;
        } catch (std::exception const& e) {
            moth::core::log::error("net: failed to listen on port {}: {}", port, e.what());
            m_impl->acceptor.reset();
            return false;
        }
    }

    void TcpServer::Close() {
        if (!m_impl->acceptor) {
            return;
        }
        std::error_code ec;
        m_impl->acceptor->close(ec);
        m_impl->acceptor.reset();
        m_impl->DropAll();
        moth::core::log::info("net: stopped listening");
    }

    bool TcpServer::IsListening() const {
        return m_impl->acceptor != nullptr;
    }

    std::uint16_t TcpServer::GetPort() const {
        if (!m_impl->acceptor) {
            return 0;
        }
        std::error_code ec;
        auto const endpoint = m_impl->acceptor->local_endpoint(ec);
        if (ec) {
            return 0;
        }
        return endpoint.port();
    }

    void TcpServer::Send(ConnectionId to, std::string_view type, nlohmann::json const& body) {
        auto conn = m_impl->FindConnection(to);
        if (!conn || !conn->socket.is_open()) {
            return;
        }
        m_impl->SendFramed(conn, m_impl->BuildPayload(type, body));
    }

    void TcpServer::Broadcast(std::string_view type, nlohmann::json const& body) {
        json const payload = m_impl->BuildPayload(type, body);
        for (auto const& conn : m_impl->connections) {
            if (conn->socket.is_open()) {
                m_impl->SendFramed(conn, payload);
            }
        }
    }

    void TcpServer::BroadcastExcept(ConnectionId except, std::string_view type, nlohmann::json const& body) {
        json const payload = m_impl->BuildPayload(type, body);
        for (auto const& conn : m_impl->connections) {
            if (conn->id == except) {
                continue;
            }
            if (conn->socket.is_open()) {
                m_impl->SendFramed(conn, payload);
            }
        }
    }

    void TcpServer::Disconnect(ConnectionId id) {
        // Snapshot — DropConnection mutates the connections vector.
        auto snapshot = m_impl->connections;
        for (auto const& conn : snapshot) {
            if (conn->id == id) {
                m_impl->DropConnection(conn);
            }
        }
    }

    std::size_t TcpServer::GetConnectionCount() const {
        return m_impl->connections.size();
    }

    void TcpServer::Poll() {
        // io_context flips into a stopped state the moment its outstanding-work
        // counter drops to zero — which happens on the first Poll() before
        // Listen() has posted anything. restart() clears that flag so later
        // completions actually dispatch.
        if (m_impl->io.stopped()) {
            m_impl->io.restart();
        }
        m_impl->io.poll();
    }

    void TcpServer::Impl::StartAccept() {
        if (!acceptor) {
            return;
        }
        auto conn = std::make_shared<Connection>(io);
        acceptor->async_accept(conn->socket,
            [this, conn](std::error_code ec) {
                HandleAccept(conn, ec);
            });
    }

    void TcpServer::Impl::HandleAccept(std::shared_ptr<Connection> const& conn, std::error_code ec) {
        if (ec) {
            // operation_aborted is the expected close path.
            if (ec != asio::error::operation_aborted) {
                moth::core::log::warn("net: accept failed: {}", ec.message());
            }
            return;
        }
        conn->id = nextId++;
        connections.push_back(conn);

        std::error_code epec;
        auto const peer = conn->socket.remote_endpoint(epec);
        if (!epec) {
            moth::core::log::info("net: client {} connected from {}:{}",
                                  conn->id, peer.address().to_string(), peer.port());
        } else {
            moth::core::log::info("net: client {} connected", conn->id);
        }
        if (connectedHandler) {
            connectedHandler(conn->id);
        }

        StartReadLength(conn);
        StartAccept();
    }

    void TcpServer::Impl::StartReadLength(std::shared_ptr<Connection> const& conn) {
        asio::async_read(conn->socket, asio::buffer(conn->lengthBuf),
            [this, conn](std::error_code ec, std::size_t /*n*/) {
                if (ec) {
                    DropConnection(conn);
                    return;
                }
                std::uint32_t const length = DecodeLength(conn->lengthBuf.data());
                if (length == 0 || length > kMaxPayloadBytes) {
                    moth::core::log::warn("net: client {} sent invalid frame length {}, dropping",
                                          conn->id, length);
                    DropConnection(conn);
                    return;
                }
                StartReadBody(conn, length);
            });
    }

    void TcpServer::Impl::StartReadBody(std::shared_ptr<Connection> const& conn, std::uint32_t length) {
        conn->bodyBuf.assign(length, 0);
        asio::async_read(conn->socket, asio::buffer(conn->bodyBuf),
            [this, conn](std::error_code ec, std::size_t /*n*/) {
                if (ec) {
                    DropConnection(conn);
                    return;
                }
                DispatchPayload(conn);
                if (conn->socket.is_open()) {
                    StartReadLength(conn);
                }
            });
    }

    void TcpServer::Impl::DispatchPayload(std::shared_ptr<Connection> const& conn) {
        json payload;
        try {
            payload = json::parse(std::string_view(conn->bodyBuf.data(), conn->bodyBuf.size()));
        } catch (json::parse_error const& e) {
            moth::core::log::warn("net: malformed JSON from client {}: {}", conn->id, e.what());
            DropConnection(conn);
            return;
        }
        auto const typeIt = payload.find("type");
        if (typeIt == payload.end() || !typeIt->is_string()) {
            moth::core::log::warn("net: client {} sent a message without a 'type' field", conn->id);
            return;
        }
        std::string const type = typeIt->get<std::string>();
        auto const it = handlers.find(type);
        if (it != handlers.end() && it->second) {
            it->second(conn->id, payload);
        } else {
            moth::core::log::debug("net: ignoring unknown message type '{}'", type);
        }
    }

    void TcpServer::Impl::SendFramed(std::shared_ptr<Connection> const& conn, json const& payload) {
        std::string const body = payload.dump();
        conn->outbox.push_back(FramePayload(body));
        if (!conn->writing) {
            DoWriteNext(conn);
        }
    }

    void TcpServer::Impl::DoWriteNext(std::shared_ptr<Connection> const& conn) {
        if (conn->outbox.empty()) {
            conn->writing = false;
            return;
        }
        conn->writing = true;
        asio::async_write(conn->socket, asio::buffer(conn->outbox.front()),
            [this, conn](std::error_code ec, std::size_t /*n*/) {
                if (ec) {
                    DropConnection(conn);
                    return;
                }
                conn->outbox.pop_front();
                DoWriteNext(conn);
            });
    }

    void TcpServer::Impl::DropConnection(std::shared_ptr<Connection> const& conn) {
        if (!conn->socket.is_open() && conn->id == TcpServer::kInvalidConnection) {
            // Already torn down and never assigned — nothing to clean up.
            return;
        }
        std::error_code ec;
        conn->socket.close(ec);
        ConnectionId const id = conn->id;
        conn->id = TcpServer::kInvalidConnection;
        if (id != TcpServer::kInvalidConnection) {
            if (disconnectedHandler) {
                disconnectedHandler(id);
            }
            moth::core::log::info("net: client {} disconnected", id);
        }
        connections.erase(
            std::remove(connections.begin(), connections.end(), conn),
            connections.end());
    }

    void TcpServer::Impl::DropAll() {
        // Snapshot — DropConnection mutates the vector.
        auto snapshot = connections;
        for (auto const& conn : snapshot) {
            DropConnection(conn);
        }
        connections.clear();
    }
}
