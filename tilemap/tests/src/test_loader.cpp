#include "moth/tilemap/tile_map_loader.h"

#include <catch2/catch_all.hpp>

using namespace moth::tilemap;

namespace {
    std::string CsvMapJson() {
        return R"({
            "width": 2, "height": 2, "tilewidth": 16, "tileheight": 16,
            "tilesets": [
                { "firstgid": 1, "name": "ts", "image": "ts.png",
                  "imagewidth": 64, "imageheight": 16, "tilewidth": 16, "tileheight": 16,
                  "columns": 4, "tilecount": 4 }
            ],
            "layers": [
                { "type": "tilelayer", "name": "ground", "width": 2, "height": 2,
                  "visible": true, "opacity": 0.5, "data": [1, 2, 0, 2147483651] }
            ]
        })";
    }

    std::string Base64MapJson() {
        return R"({
            "width": 2, "height": 2, "tilewidth": 16, "tileheight": 16,
            "tilesets": [
                { "firstgid": 1, "name": "ts", "image": "ts.png",
                  "imagewidth": 64, "imageheight": 16, "tilewidth": 16, "tileheight": 16,
                  "columns": 4, "tilecount": 4 }
            ],
            "layers": [
                { "type": "tilelayer", "name": "ground", "width": 2, "height": 2,
                  "encoding": "base64", "data": "AQAAAAIAAAADAAAABAAAAA==" }
            ]
        })";
    }
}

TEST_CASE("Loader: CSV map loads dimensions, tileset, and tiles", "[tilemap][loader]") {
    TileMap const map = LoadTileMap(CsvMapJson());

    REQUIRE(map.width == 2);
    REQUIRE(map.height == 2);
    REQUIRE(map.tileWidth == 16);
    REQUIRE(map.tileHeight == 16);

    REQUIRE(map.GetTilesetCount() == 1);
    REQUIRE(map.GetTileset(0).firstGid == 1);
    REQUIRE(map.GetTileset(0).columns == 4);
    REQUIRE(map.GetTileset(0).tileCount == 4);
    REQUIRE(map.GetTileset(0).imagePath == "ts.png");

    REQUIRE(map.GetLayerCount() == 1);
    REQUIRE(map.GetLayer(0).name == "ground");
    REQUIRE(map.GetLayer(0).opacity == Catch::Approx(0.5f));
    REQUIRE(map.GetTile(0, 0, 0).id == 1);
    REQUIRE(map.GetTile(0, 1, 0).id == 2);
    REQUIRE(map.GetTile(0, 0, 1).IsEmpty());
    // 2147483651 = 0x80000003 -> id 3, horizontal flip.
    REQUIRE(map.GetTile(0, 1, 1).id == 3);
    REQUIRE(map.GetTile(0, 1, 1).flipHorizontal);
}

TEST_CASE("Loader: base64 tile data decodes little-endian GIDs", "[tilemap][loader]") {
    TileMap const map = LoadTileMap(Base64MapJson());

    REQUIRE(map.GetLayerCount() == 1);
    REQUIRE(map.GetTile(0, 0, 0).id == 1);
    REQUIRE(map.GetTile(0, 1, 0).id == 2);
    REQUIRE(map.GetTile(0, 0, 1).id == 3);
    REQUIRE(map.GetTile(0, 1, 1).id == 4);
}

TEST_CASE("Loader: multiple layers preserve order and visibility", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 1, "height": 1, "tilewidth": 16, "tileheight": 16,
        "layers": [
            { "type": "tilelayer", "name": "back", "data": [1], "visible": false, "opacity": 0.25 },
            { "type": "tilelayer", "name": "front", "data": [2], "visible": true, "opacity": 1.0 }
        ]
    })";

    TileMap const map = LoadTileMap(json);
    REQUIRE(map.GetLayerCount() == 2);
    REQUIRE(map.GetLayer(0).name == "back");
    REQUIRE_FALSE(map.GetLayer(0).visible);
    REQUIRE(map.GetLayer(0).opacity == Catch::Approx(0.25f));
    REQUIRE(map.GetLayer(1).name == "front");
    REQUIRE(map.GetLayer(1).visible);
}

TEST_CASE("Loader: object layers are skipped", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 1, "height": 1, "tilewidth": 16, "tileheight": 16,
        "layers": [
            { "type": "objectgroup", "name": "colliders", "objects": [] },
            { "type": "tilelayer", "name": "ground", "data": [1] }
        ]
    })";

    TileMap const map = LoadTileMap(json);
    REQUIRE(map.GetLayerCount() == 1);
    REQUIRE(map.GetLayer(0).name == "ground");
}

TEST_CASE("Loader: malformed input throws", "[tilemap][loader]") {
    REQUIRE_THROWS(LoadTileMap(R"({"width": 1})"));
    REQUIRE_THROWS(LoadTileMap("not json"));
}

TEST_CASE("Loader: compressed tile data is rejected", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 1, "height": 1, "tilewidth": 16, "tileheight": 16,
        "layers": [
            { "type": "tilelayer", "data": "AAAA", "encoding": "base64", "compression": "zlib" }
        ]
    })";
    REQUIRE_THROWS(LoadTileMap(json));
}
