# moth::net

**Package** `moth_net` · **Namespace** `moth::net` · **Umbrella** `<moth/net/net.h>` · **CMake target** `moth::net`

A poll-driven, host-authoritative TCP module. `TcpServer`/`TcpClient` speak a
length-prefixed JSON wire format (4-byte big-endian length + UTF-8 JSON with a
`type` tag and a per-sender `seq`), dispatch incoming messages to handlers
registered by `type` string, and drain the asio `io_context` on the app thread
via `Poll()` — no background threads. Depends on `moth_core` + `asio` (1.30.2) +
`nlohmann_json`.

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/net/net.h>

using namespace moth::net;

TcpServer server;
server.SetHandler("Ping", [&](TcpServer::ConnectionId from, nlohmann::json const& p) {
    server.Send(from, "Pong", nlohmann::json{{"answer", p["value"].get<int>() + 1}});
});
server.Listen(47624);                       // 0 = ephemeral port; GetPort() reads it

TcpClient client;
client.SetHandler("Pong", [&](nlohmann::json const& p) { /* ... */ });
client.Connect("127.0.0.1", 47624);
client.Send("Ping", nlohmann::json{{"value", 41}});

// In the game loop, tick both once per frame:
server.Poll();
client.Poll();
```

## Model

- **Star topology.** One host runs a `TcpServer`, every player runs a `TcpClient`,
  and the host is the authority.
- **Poll-driven.** Nothing runs in the background. Accepts, reads, writes, and your
  handlers all run inside `Poll()` on the thread that calls it, so handlers can
  touch game state without locking. Call `Poll()` once per tick.
- **Messages by type.** Every message has a string `type`. Register one handler per
  type; messages without a handler are ignored.

## Wire format

Each message is a 4-byte big-endian length followed by that many bytes of UTF-8
JSON. The JSON object carries:

- `type`: the message type string;
- `seq`: a monotonic per-sender counter starting at 0;
- the fields of the body you passed to `Send`, as flat key/value pairs.

Handlers receive this whole payload, so `p["value"]` reads a body field directly.
A single payload is capped at `kMaxPayloadBytes` (64 KiB), which protects the
process from a buggy or hostile sender. `frame.h` exposes `FramePayload` and
`DecodeLength` for anything that needs to speak the format directly.

## TcpServer

| Call | Does |
|---|---|
| `Listen(port)` | Starts accepting; 0 picks an ephemeral port. Returns `false` if binding fails |
| `GetPort()` / `IsListening()` | The bound port (0 when not listening) / whether it's accepting |
| `Close()` | Stops accepting and closes every connection |
| `SetHandler(type, handler)` | Handles a message type with `(ConnectionId from, json payload)`; a null handler removes it |
| `SetConnectedHandler` / `SetDisconnectedHandler` | Called with the `ConnectionId` |
| `Send(to, type, body)` | Sends to one client; no-op for an unknown or closed id |
| `Broadcast(type, body)` / `BroadcastExcept(id, type, body)` | Sends to everyone / everyone but one |
| `Disconnect(id)` | Closes one connection, firing the disconnect handler |
| `GetConnectionCount()` | Number of live connections |
| `Poll()` | Runs ready I/O and handlers |

Connection ids are assigned in accept order starting at 1 and are never reused
within a server's lifetime. `kInvalidConnection` is 0.

## TcpClient

| Call | Does |
|---|---|
| `Connect(host, port)` | Starts an async connect. `host` must be an IPv4 or IPv6 literal. Aborts any previous connection |
| `GetState()` | `Idle`, `Connecting`, `Connected`, or `Failed` |
| `LastFailureReason()` | Why the last connection failed |
| `SetHandler(type, handler)` | Handles a message type with `(json payload)` |
| `SetConnectedHandler` / `SetDisconnectedHandler` / `SetFailedHandler` | Lifecycle callbacks |
| `Send(type, body)` | Sends a message; no-op unless connected |
| `Disconnect()` | Tears down the connection; safe in any state |
| `Poll()` | Runs ready I/O and handlers; safe in any state |

When an established connection drops, the disconnected handler fires, then the
failed handler. A refused or rejected connect fires only the failed handler.

```cpp
client.SetConnectedHandler([&] { client.Send("Join", {{"name", playerName}}); });
client.SetFailedHandler([](std::string_view reason) { ShowError(reason); });
```

Neither class can be copied or moved.

## Using the package

```python
def requirements(self):
    self.requires("moth_net/0.1.0")
```

```cmake
find_package(moth_net REQUIRED)
target_link_libraries(my_game PRIVATE moth::net)
```

## Tests

```bash
cd modules/net/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```
