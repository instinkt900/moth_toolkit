#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

namespace moth::net {
    /**
     * @brief A poll-driven TCP server for host-authoritative (star) play.
     *
     * Accepts clients, frames length-prefixed JSON messages (see frame.h), and
     * dispatches each incoming message to the handler registered under its
     * "type" string. Outbound messages are addressed by the connection id the
     * server assigned on accept, or broadcast to everyone.
     *
     * The asio io_context is drained via @ref Poll — call it once per app tick
     * so accept/read/write completions run on the app thread. No background
     * thread is spawned.
     */
    class TcpServer {
    public:
        /// @brief Opaque handle for one connected client. Assigned in accept
        /// order, starting at 1; never reused within a server's lifetime.
        using ConnectionId = std::uint32_t;
        static constexpr ConnectionId kInvalidConnection = 0;

        /// @brief Invoked with the sender id and the full parsed JSON payload
        /// (which carries "type"/"seq" plus any caller-supplied body) for a
        /// message whose "type" was registered via @ref SetHandler.
        using MessageHandler = std::function<void(ConnectionId from, nlohmann::json const& payload)>;
        /// @brief Invoked when a client connects / disconnects.
        using ConnectionHandler = std::function<void(ConnectionId id)>;

        TcpServer();
        ~TcpServer();

        TcpServer(TcpServer const&) = delete;
        TcpServer(TcpServer&&) = delete;
        TcpServer& operator=(TcpServer const&) = delete;
        TcpServer& operator=(TcpServer&&) = delete;

        /// @brief Registers (or replaces, or with a null handler removes) the
        /// handler for messages whose "type" tag is @p type.
        void SetHandler(std::string_view type, MessageHandler handler);

        /// @brief Registers callbacks for connection lifecycle events.
        void SetConnectedHandler(ConnectionHandler handler);
        void SetDisconnectedHandler(ConnectionHandler handler);

        /// @brief Begins accepting on @p port (0 picks an ephemeral port).
        /// Returns @c false if the socket could not be bound.
        bool Listen(std::uint16_t port);

        /// @brief Stops accepting and closes every live connection. No-op if not listening.
        void Close();

        /// @brief Returns @c true while the accept loop is active.
        bool IsListening() const;

        /// @brief Returns the bound local port, or 0 when not listening.
        std::uint16_t GetPort() const;

        /// @brief Sends a message with the given @p type and @p body (an object,
        /// or null) to @p to. No-op for an unknown or closed connection.
        void Send(ConnectionId to, std::string_view type, nlohmann::json const& body = {});

        /// @brief Sends a message to every connected client.
        void Broadcast(std::string_view type, nlohmann::json const& body = {});

        /// @brief Sends a message to every connected client except @p except.
        void BroadcastExcept(ConnectionId except, std::string_view type, nlohmann::json const& body = {});

        /// @brief Closes the connection held by @p id, firing the disconnect path.
        void Disconnect(ConnectionId id);

        /// @brief Returns the number of live connections.
        std::size_t GetConnectionCount() const;

        /// @brief Drains ready io_context completions. Safe to call when not listening.
        void Poll();

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
