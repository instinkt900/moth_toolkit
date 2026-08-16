#include "moth/tilemap/tile_map.h"

#include <catch2/catch_all.hpp>

using namespace moth::tilemap;

namespace {
    Tileset MakeTileset(int firstGid, int columns, int tileCount) {
        Tileset ts;
        ts.firstGid = firstGid;
        ts.tileWidth = 16;
        ts.tileHeight = 16;
        ts.columns = columns;
        ts.tileCount = tileCount;
        return ts;
    }

    TileMap MakeMap(int width, int height, int tileWidth = 16, int tileHeight = 16) {
        TileMap map;
        map.width = width;
        map.height = height;
        map.tileWidth = tileWidth;
        map.tileHeight = tileHeight;
        return map;
    }
}

TEST_CASE("TileMap: WorldToTile and TileToWorld round-trip", "[tilemap][map]") {
    TileMap map = MakeMap(4, 4);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            auto const world = map.TileToWorld(x, y);
            auto const tile = map.WorldToTile(world);
            REQUIRE(tile.x == x);
            REQUIRE(tile.y == y);
        }
    }
}

TEST_CASE("TileMap: WorldToTile handles fractional and negative positions", "[tilemap][map]") {
    TileMap map = MakeMap(4, 4);
    REQUIRE(map.WorldToTile({ 15.9f, 15.9f }).x == 0);
    REQUIRE(map.WorldToTile({ 16.0f, 16.0f }).x == 1);
    REQUIRE(map.WorldToTile({ -0.1f, -0.1f }).x == -1);
}

TEST_CASE("Tileset: GetTileRect computes the atlas rect", "[tilemap][map]") {
    Tileset ts = MakeTileset(1, 4, 8);
    // Tile 0 -> column 0, row 0.
    REQUIRE(ts.GetTileRect(0) == MakeRect(0, 0, 16, 16));
    // Tile 4 -> column 0, row 1.
    REQUIRE(ts.GetTileRect(4) == MakeRect(0, 16, 16, 16));
    // Tile 5 -> column 1, row 1.
    REQUIRE(ts.GetTileRect(5) == MakeRect(16, 16, 16, 16));
}

TEST_CASE("Tileset: GetTileRect honours margin and spacing", "[tilemap][map]") {
    Tileset ts = MakeTileset(1, 4, 8);
    ts.margin = 2;
    ts.spacing = 4;
    REQUIRE(ts.GetTileRect(0) == MakeRect(2, 2, 16, 16));
    REQUIRE(ts.GetTileRect(1) == MakeRect(2 + 16 + 4, 2, 16, 16));
}

TEST_CASE("Tileset: ContainsGid and LocalId", "[tilemap][map]") {
    Tileset ts = MakeTileset(5, 4, 4); // owns gids 5..8
    REQUIRE(ts.ContainsGid(5));
    REQUIRE(ts.ContainsGid(8));
    REQUIRE_FALSE(ts.ContainsGid(4));
    REQUIRE_FALSE(ts.ContainsGid(9));
    REQUIRE(ts.LocalId(5) == 0);
    REQUIRE(ts.LocalId(8) == 3);
}

TEST_CASE("TileMap: FindTileset picks the owning tileset", "[tilemap][map]") {
    TileMap map = MakeMap(1, 1);
    map.tilesets.push_back(MakeTileset(1, 4, 4));
    map.tilesets.push_back(MakeTileset(5, 4, 4));

    REQUIRE(map.FindTileset(1) == &map.tilesets[0]);
    REQUIRE(map.FindTileset(4) == &map.tilesets[0]);
    REQUIRE(map.FindTileset(5) == &map.tilesets[1]);
    REQUIRE(map.FindTileset(9) == nullptr);
}

TEST_CASE("TileMap: GetTile and GetTileAtWorld query layers", "[tilemap][map]") {
    TileMap map = MakeMap(2, 2);
    Layer layer;
    layer.width = 2;
    layer.height = 2;
    layer.tiles = { TileId::FromGid(1), TileId::FromGid(2), TileId::FromGid(0), TileId::FromGid(3) };
    map.layers.push_back(layer);

    REQUIRE(map.GetTile(0, 0, 0).id == 1);
    REQUIRE(map.GetTile(0, 1, 0).id == 2);
    REQUIRE(map.GetTile(0, 0, 1).IsEmpty());
    REQUIRE(map.GetTile(0, 1, 1).id == 3);

    // Out of bounds -> empty.
    REQUIRE(map.GetTile(0, -1, 0).IsEmpty());
    REQUIRE(map.GetTile(0, 2, 0).IsEmpty());
    REQUIRE(map.GetTile(1, 0, 0).IsEmpty());

    // World query (16px tiles).
    REQUIRE(map.GetTileAtWorld(0, { 8.0f, 8.0f }).id == 1);
    REQUIRE(map.GetTileAtWorld(0, { 24.0f, 8.0f }).id == 2);
}

TEST_CASE("Layer: SetTile writes and respects bounds", "[tilemap][map]") {
    Layer layer;
    layer.width = 2;
    layer.height = 2;
    layer.tiles.resize(4);

    layer.SetTile(1, 1, TileId::FromGid(9));
    REQUIRE(layer.GetTile(1, 1).id == 9);

    layer.SetTile(5, 5, TileId::FromGid(9)); // ignored
    REQUIRE(layer.GetTile(5, 5).IsEmpty());
}

TEST_CASE("ResolveTileId: non-animated tiles resolve to themselves", "[tilemap][animation]") {
    Tileset ts = MakeTileset(1, 4, 8);
    REQUIRE(ResolveTileId(ts, 3, 0) == 3);
}

TEST_CASE("ResolveTileId: animated tiles cycle through frames", "[tilemap][animation]") {
    Tileset ts = MakeTileset(1, 4, 8);
    ts.animations[0] = { { 0, 100 }, { 1, 100 }, { 2, 100 } };

    REQUIRE(ResolveTileId(ts, 0, 0) == 0);
    REQUIRE(ResolveTileId(ts, 0, 99) == 0);
    REQUIRE(ResolveTileId(ts, 0, 100) == 1);
    REQUIRE(ResolveTileId(ts, 0, 200) == 2);
    REQUIRE(ResolveTileId(ts, 0, 300) == 0); // wraps
}

TEST_CASE("ResolveTileId: respects per-frame durations", "[tilemap][animation]") {
    Tileset ts = MakeTileset(1, 4, 8);
    ts.animations[0] = { { 0, 1000 }, { 1, 100 } };

    REQUIRE(ResolveTileId(ts, 0, 500) == 0);
    REQUIRE(ResolveTileId(ts, 0, 1000) == 1);
    REQUIRE(ResolveTileId(ts, 0, 1099) == 1);
    REQUIRE(ResolveTileId(ts, 0, 1100) == 0); // wraps
}

TEST_CASE("Layer: infinite chunks resolve via floor division", "[tilemap][infinite]") {
    Layer layer;
    layer.infinite = true;
    Chunk chunk;
    chunk.x = 0;
    chunk.y = 0;
    chunk.tiles[0] = TileId::FromGid(1);  // tile (0, 0)
    chunk.tiles[1] = TileId::FromGid(2);  // tile (1, 0)
    chunk.tiles[16] = TileId::FromGid(3); // tile (0, 1)
    layer.chunks.push_back(chunk);

    REQUIRE(layer.GetTile(0, 0).id == 1);
    REQUIRE(layer.GetTile(1, 0).id == 2);
    REQUIRE(layer.GetTile(0, 1).id == 3);
    REQUIRE(layer.GetTile(2, 0).IsEmpty());   // empty tile within the chunk
    REQUIRE(layer.GetTile(20, 0).IsEmpty());  // no chunk at (1, 0)
}

TEST_CASE("Layer: infinite chunks handle negative coordinates", "[tilemap][infinite]") {
    Layer layer;
    layer.infinite = true;
    Chunk chunk;
    chunk.x = -1;
    chunk.y = 0;
    chunk.tiles[15] = TileId::FromGid(7); // tile (-1, 0) -> chunk (-1, 0), local (15, 0)
    layer.chunks.push_back(chunk);

    REQUIRE(layer.GetTile(-1, 0).id == 7);
    REQUIRE(layer.GetTile(0, 0).IsEmpty()); // chunk (0, 0) absent
}
