#include "moth/net/tcp_client.h"

#include "moth/net/frame.h"

#include <moth/core/log.h>

#include <asio.hpp>

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

    struct TcpClient::Impl {
        asio::io_context io;
        asio::ip::tcp::socket socket{io};
        std::array<unsigned char, kLengthPrefixBytes> lengthBuf{};
        std::vector<char> bodyBuf;
        std::deque<std::string> outbox;
        bool writing = false;

        State state = State::Idle;
        std::string failureReason;
        std::uint32_t seq = 0;

        std::unordered_map<std::string, MessageHandler> handlers;
        ConnectedHandler connectedHandler;
        FailedHandler failedHandler;
        DisconnectedHandler disconnectedHandler;

        void Reset();
        void Fail(std::string_view reason);

        void StartReadLength();
        void StartReadBody(std::uint32_t length);
        void DispatchPayload();

        void SendFramed(json const& payload);
        void DoWriteNext();
    };

    TcpClient::TcpClient() : m_impl(std::make_unique<Impl>()) {}

    TcpClient::~TcpClient() {
        Disconnect();
    }

    void TcpClient::SetHandler(std::string_view type, MessageHandler handler) {
        if (handler) {
            m_impl->handlers[std::string(type)] = std::move(handler);
        } else {
            m_impl->handlers.erase(std::string(type));
        }
    }

    void TcpClient::SetConnectedHandler(ConnectedHandler handler) {
        m_impl->connectedHandler = std::move(handler);
    }

    void TcpClient::SetFailedHandler(FailedHandler handler) {
        m_impl->failedHandler = std::move(handler);
    }

    void TcpClient::SetDisconnectedHandler(DisconnectedHandler handler) {
        m_impl->disconnectedHandler = std::move(handler);
    }

    TcpClient::State TcpClient::GetState() const {
        return m_impl->state;
    }

    std::string const& TcpClient::LastFailureReason() const {
        return m_impl->failureReason;
    }

    void TcpClient::Connect(std::string_view host, std::uint16_t port) {
        Disconnect();
        m_impl->state = State::Connecting;
        m_impl->failureReason.clear();
        std::string const hostCopy(host);

        asio::ip::tcp::endpoint endpoint;
        std::error_code parseEc;
        auto const addr = asio::ip::make_address(hostCopy, parseEc);
        if (parseEc) {
            m_impl->Fail("invalid IP address");
            return;
        }
        endpoint.address(addr);
        endpoint.port(port);

        moth::core::log::info("net: connecting to {}:{}", hostCopy, port);
        m_impl->socket.async_connect(endpoint,
            [impl = m_impl.get()](std::error_code ec) {
                if (ec) {
                    impl->Fail(ec.message());
                    return;
                }
                impl->state = State::Connected;
                impl->StartReadLength();
                if (impl->connectedHandler) {
                    impl->connectedHandler();
                }
            });
    }

    void TcpClient::Disconnect() {
        if (m_impl->state == State::Idle) {
            return;
        }
        m_impl->Reset();
        m_impl->state = State::Idle;
    }

    void TcpClient::Send(std::string_view type, nlohmann::json const& body) {
        if (m_impl->state != State::Connected) {
            return;
        }
        json payload = body.is_object() ? body : json::object();
        if (!body.is_object() && !body.is_null()) {
            payload["body"] = body;
        }
        payload["type"] = type;
        payload["seq"] = m_impl->seq++;
        m_impl->SendFramed(payload);
    }

    void TcpClient::Poll() {
        // Mirror of TcpServer::Poll — see the comment there.
        if (m_impl->io.stopped()) {
            m_impl->io.restart();
        }
        m_impl->io.poll();
    }

    void TcpClient::Impl::Reset() {
        std::error_code ec;
        socket.close(ec);
        socket = asio::ip::tcp::socket(io);
        outbox.clear();
        writing = false;
        bodyBuf.clear();
        seq = 0;
    }

    void TcpClient::Impl::Fail(std::string_view reason) {
        failureReason.assign(reason);
        bool const wasConnected = state == State::Connected;
        Reset();
        state = State::Failed;
        moth::core::log::warn("net: connection failed: {}", failureReason);
        if (wasConnected && disconnectedHandler) {
            disconnectedHandler();
        }
        if (failedHandler) {
            failedHandler(failureReason);
        }
    }

    void TcpClient::Impl::StartReadLength() {
        asio::async_read(socket, asio::buffer(lengthBuf),
            [this](std::error_code ec, std::size_t /*n*/) {
                if (ec) {
                    Fail(ec.message());
                    return;
                }
                std::uint32_t const length = DecodeLength(lengthBuf.data());
                if (length == 0 || length > kMaxPayloadBytes) {
                    Fail("invalid frame length from host");
                    return;
                }
                StartReadBody(length);
            });
    }

    void TcpClient::Impl::StartReadBody(std::uint32_t length) {
        bodyBuf.assign(length, 0);
        asio::async_read(socket, asio::buffer(bodyBuf),
            [this](std::error_code ec, std::size_t /*n*/) {
                if (ec) {
                    Fail(ec.message());
                    return;
                }
                DispatchPayload();
                if (socket.is_open()) {
                    StartReadLength();
                }
            });
    }

    void TcpClient::Impl::DispatchPayload() {
        json payload;
        try {
            payload = json::parse(std::string_view(bodyBuf.data(), bodyBuf.size()));
        } catch (json::parse_error const& e) {
            Fail(std::string("malformed JSON from host: ") + e.what());
            return;
        }
        auto const typeIt = payload.find("type");
        if (typeIt == payload.end() || !typeIt->is_string()) {
            return;
        }
        std::string const type = typeIt->get<std::string>();
        auto const it = handlers.find(type);
        if (it != handlers.end() && it->second) {
            it->second(payload);
        } else {
            moth::core::log::debug("net: ignoring unknown message type '{}'", type);
        }
    }

    void TcpClient::Impl::SendFramed(json const& payload) {
        std::string const body = payload.dump();
        outbox.push_back(FramePayload(body));
        if (!writing) {
            DoWriteNext();
        }
    }

    void TcpClient::Impl::DoWriteNext() {
        if (outbox.empty()) {
            writing = false;
            return;
        }
        writing = true;
        asio::async_write(socket, asio::buffer(outbox.front()),
            [this](std::error_code ec, std::size_t /*n*/) {
                if (ec) {
                    Fail(ec.message());
                    return;
                }
                outbox.pop_front();
                DoWriteNext();
            });
    }
}
