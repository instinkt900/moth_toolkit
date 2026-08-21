#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

namespace moth::net {
    // Wire format: a 4-byte big-endian length prefix followed by a UTF-8 JSON
    // payload. Every payload carries a string "type" tag and a monotonic
    // per-sender "seq" (0-based). Bodies are flat key/value pairs.

    /// @brief Size of the big-endian length prefix on the wire.
    constexpr std::size_t kLengthPrefixBytes = 4;

    /// @brief Cap on a single payload so a hostile or buggy sender cannot drag
    /// the process down. Lobby/message payloads are tiny.
    constexpr std::uint32_t kMaxPayloadBytes = 64 * 1024;

    /// @brief Builds a framed wire buffer: 4-byte big-endian length + payload.
    inline std::string FramePayload(std::string_view payload) {
        std::string out;
        out.resize(kLengthPrefixBytes + payload.size());
        auto const len = static_cast<std::uint32_t>(payload.size());
        out[0] = static_cast<char>((len >> 24) & 0xFFu);
        out[1] = static_cast<char>((len >> 16) & 0xFFu);
        out[2] = static_cast<char>((len >> 8) & 0xFFu);
        out[3] = static_cast<char>(len & 0xFFu);
        std::memcpy(out.data() + kLengthPrefixBytes, payload.data(), payload.size());
        return out;
    }

    /// @brief Decodes a 4-byte big-endian length prefix.
    inline std::uint32_t DecodeLength(unsigned char const* bytes) {
        return (static_cast<std::uint32_t>(bytes[0]) << 24)
             | (static_cast<std::uint32_t>(bytes[1]) << 16)
             | (static_cast<std::uint32_t>(bytes[2]) << 8)
             | static_cast<std::uint32_t>(bytes[3]);
    }
}
