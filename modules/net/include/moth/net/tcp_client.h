#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

namespace moth::net {
    /**
     * @brief A poll-driven TCP client, peer of @ref TcpServer.
     *
     * Speaks the same length-prefixed JSON framing and dispatches incoming
     * messages by their "type" string. Like the server, it is driven by
     * periodic @ref Poll calls on the app thread.
     */
    class TcpClient {
    public:
        enum class State {
            Idle,         ///< Not connecting.
            Connecting,   ///< Socket connect in flight.
            Connected,    ///< TCP connection established.
            Failed,       ///< Connection refused, rejected, or dropped.
        };

        /// @brief Invoked with the full parsed JSON payload for a message whose
        /// "type" was registered via @ref SetHandler.
        using MessageHandler = std::function<void(nlohmann::json const& payload)>;
        using ConnectedHandler = std::function<void()>;
        using FailedHandler = std::function<void(std::string_view reason)>;
        using DisconnectedHandler = std::function<void()>;

        TcpClient();
        ~TcpClient();

        TcpClient(TcpClient const&) = delete;
        TcpClient(TcpClient&&) = delete;
        TcpClient& operator=(TcpClient const&) = delete;
        TcpClient& operator=(TcpClient&&) = delete;

        /// @brief Registers (or replaces, or with a null handler removes) the
        /// handler for messages whose "type" tag is @p type.
        void SetHandler(std::string_view type, MessageHandler handler);

        /// @brief Registers lifecycle callbacks. @c DisconnectedHandler fires
        /// when an established connection drops; @c FailedHandler fires on any
        /// failure (including a dropped connection, after the disconnect path).
        void SetConnectedHandler(ConnectedHandler handler);
        void SetFailedHandler(FailedHandler handler);
        void SetDisconnectedHandler(DisconnectedHandler handler);

        State GetState() const;
        std::string const& LastFailureReason() const;

        /// @brief Begins an async connect to @p host (IPv4/IPv6 literal) on
        /// @p port. Aborts any prior connection.
        void Connect(std::string_view host, std::uint16_t port);

        /// @brief Tears down the connection. Safe to call in any state.
        void Disconnect();

        /// @brief Sends a message with the given @p type and @p body (an object,
        /// or null). No-op unless connected.
        void Send(std::string_view type, nlohmann::json const& body = {});

        /// @brief Drains ready io_context completions. Safe to call in any state.
        void Poll();

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
