#include "moth/net/net.h"

#include <catch2/catch_all.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

using namespace moth::net;

namespace {
    // Polls server+client until `done` returns true or the timeout elapses.
    // Poll() is non-blocking, so a tiny sleep lets the OS make progress on the
    // loopback socket between polls.
    template <typename Done>
    bool WaitUntil(TcpServer& server, TcpClient& client, Done done, int maxMs = 3000) {
        auto const start = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start).count() < maxMs) {
            server.Poll();
            client.Poll();
            if (done()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return false;
    }

    // Same, but ticks two clients (every live connection must be polled).
    template <typename Done>
    bool WaitUntil(TcpServer& server, TcpClient& a, TcpClient& b, Done done, int maxMs = 3000) {
        auto const start = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start).count() < maxMs) {
            server.Poll();
            a.Poll();
            b.Poll();
            if (done()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return false;
    }
}

TEST_CASE("FramePayload writes a big-endian length prefix", "[net][frame]") {
    std::string const payload = "{\"type\":\"Ping\"}";
    std::string const framed = FramePayload(payload);

    REQUIRE(framed.size() == kLengthPrefixBytes + payload.size());

    auto const* bytes = reinterpret_cast<unsigned char const*>(framed.data());
    REQUIRE(DecodeLength(bytes) == payload.size());
    REQUIRE(std::string(framed.begin() + kLengthPrefixBytes, framed.end()) == payload);
}

TEST_CASE("DecodeLength reads all four prefix bytes", "[net][frame]") {
    unsigned char const bytes[4] = { 0x00, 0x01, 0x02, 0x03 };
    REQUIRE(DecodeLength(bytes) == 0x00010203u);
}

TEST_CASE("Client and server exchange typed messages over loopback", "[net][tcp]") {
    TcpServer server;
    REQUIRE(server.Listen(0));
    REQUIRE(server.IsListening());

    std::uint16_t const port = server.GetPort();
    REQUIRE(port != 0);

    TcpServer::ConnectionId connectedId = TcpServer::kInvalidConnection;
    bool serverGotPing = false;

    server.SetConnectedHandler([&](TcpServer::ConnectionId id) {
        connectedId = id;
    });
    server.SetHandler("Ping", [&](TcpServer::ConnectionId from, nlohmann::json const& payload) {
        REQUIRE(payload["value"].get<int>() == 42);
        REQUIRE(payload["type"].get<std::string>() == "Ping");
        serverGotPing = true;
        server.Send(from, "Pong", nlohmann::json{{"answer", 43}});
    });

    TcpClient client;
    bool clientGotPong = false;
    client.SetHandler("Pong", [&](nlohmann::json const& payload) {
        REQUIRE(payload["answer"].get<int>() == 43);
        clientGotPong = true;
    });

    client.Connect("127.0.0.1", port);

    REQUIRE(WaitUntil(server, client, [&] {
        return client.GetState() == TcpClient::State::Connected;
    }));
    REQUIRE(server.GetConnectionCount() == 1);
    REQUIRE(connectedId != TcpServer::kInvalidConnection);

    client.Send("Ping", nlohmann::json{{"value", 42}});

    REQUIRE(WaitUntil(server, client, [&] {
        return serverGotPing && clientGotPong;
    }));

    client.Disconnect();
    server.Close();
}

TEST_CASE("Server broadcasts to all connections except the excluded one", "[net][tcp]") {
    TcpServer server;
    REQUIRE(server.Listen(0));
    std::uint16_t const port = server.GetPort();

    int bounces = 0;
    server.SetHandler("Bounce", [&](TcpServer::ConnectionId from, nlohmann::json const& payload) {
        server.BroadcastExcept(from, "Bounced", payload["value"]);
        (void)payload;
    });

    TcpClient a;
    TcpClient b;
    int aGot = 0;
    int bGot = 0;
    a.SetHandler("Bounced", [&](nlohmann::json const&) { ++aGot; });
    b.SetHandler("Bounced", [&](nlohmann::json const&) { ++bGot; });

    a.Connect("127.0.0.1", port);
    b.Connect("127.0.0.1", port);

    REQUIRE(WaitUntil(server, a, [&] {
        return a.GetState() == TcpClient::State::Connected;
    }));
    REQUIRE(WaitUntil(server, b, [&] {
        return b.GetState() == TcpClient::State::Connected;
    }));
    REQUIRE(server.GetConnectionCount() == 2);

    a.Send("Bounce", nlohmann::json{{"value", 7}});

    REQUIRE(WaitUntil(server, a, b, [&] {
        return aGot == 0 && bGot == 1;
    }));

    a.Disconnect();
    b.Disconnect();
    server.Close();
}

TEST_CASE("Client reports failure when connecting to a closed port", "[net][tcp]") {
    TcpServer server;
    REQUIRE(server.Listen(0));
    std::uint16_t const port = server.GetPort();
    server.Close();

    TcpClient client;
    std::string reason;
    client.SetFailedHandler([&](std::string_view r) { reason.assign(r); });

    client.Connect("127.0.0.1", port);
    REQUIRE(WaitUntil(server, client, [&] {
        return client.GetState() == TcpClient::State::Failed;
    }));
    REQUIRE(!reason.empty());
}
