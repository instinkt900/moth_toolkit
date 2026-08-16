#include "moth/tilemap/tile.h"

#include <catch2/catch_all.hpp>

using namespace moth::tilemap;

TEST_CASE("TileId: FromGid unpacks the id", "[tilemap][tile]") {
    TileId const tile = TileId::FromGid(7u);
    REQUIRE(tile.id == 7u);
    REQUIRE_FALSE(tile.flipHorizontal);
    REQUIRE_FALSE(tile.flipVertical);
    REQUIRE_FALSE(tile.flipDiagonal);
}

TEST_CASE("TileId: FromGid unpacks flip flags", "[tilemap][tile]") {
    TileId const h = TileId::FromGid(0x80000005u);
    REQUIRE(h.id == 5u);
    REQUIRE(h.flipHorizontal);
    REQUIRE_FALSE(h.flipVertical);
    REQUIRE_FALSE(h.flipDiagonal);

    TileId const v = TileId::FromGid(0x40000006u);
    REQUIRE(v.id == 6u);
    REQUIRE(v.flipVertical);

    TileId const d = TileId::FromGid(0x20000007u);
    REQUIRE(d.id == 7u);
    REQUIRE(d.flipDiagonal);
}

TEST_CASE("TileId: ToGid round-trips a GID", "[tilemap][tile]") {
    for (std::uint32_t gid : { 0u, 1u, 5u, 0x80000005u, 0x40000006u, 0x20000007u, 0xE0000009u }) {
        REQUIRE(TileId::FromGid(gid).ToGid() == gid);
    }
}

TEST_CASE("TileId: GID 0 is the empty tile", "[tilemap][tile]") {
    TileId const empty = TileId::FromGid(0u);
    REQUIRE(empty.IsEmpty());
    REQUIRE_FALSE(TileId::FromGid(1u).IsEmpty());
}

TEST_CASE("TileId: equality compares id and flags", "[tilemap][tile]") {
    REQUIRE(TileId::FromGid(3u) == TileId::FromGid(3u));
    REQUIRE(TileId::FromGid(0x80000003u) != TileId::FromGid(3u));
}
