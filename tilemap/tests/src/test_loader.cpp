#include "moth/tilemap/tile_map_loader.h"

#include <catch2/catch_all.hpp>

#include <filesystem>
#include <fstream>

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

TEST_CASE("Loader: object layers are parsed alongside tile layers", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 1, "height": 1, "tilewidth": 16, "tileheight": 16,
        "layers": [
            { "type": "objectgroup", "name": "colliders", "objects": [] },
            { "type": "tilelayer", "name": "ground", "data": [1] }
        ]
    })";

    TileMap const map = LoadTileMap(json);
    REQUIRE(map.GetLayerCount() == 1);        // only the tile layer here
    REQUIRE(map.GetLayer(0).name == "ground");
    REQUIRE(map.GetObjectLayerCount() == 1);  // object layer stored separately
    REQUIRE(map.GetObjectLayer(0).name == "colliders");
}

TEST_CASE("Loader: object shapes and fields load", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 4, "height": 4, "tilewidth": 16, "tileheight": 16,
        "layers": [
            { "type": "objectgroup", "name": "objs", "objects": [
                { "id": 1, "name": "spawn", "type": "player", "x": 10.5, "y": 20.0,
                  "width": 32, "height": 32, "rotation": 45 },
                { "id": 2, "name": "zone", "ellipse": true, "x": 0, "y": 0, "width": 16, "height": 16 },
                { "id": 3, "name": "tri", "x": 64, "y": 64,
                  "polygon": [ { "x": 0, "y": 0 }, { "x": 16, "y": 0 }, { "x": 16, "y": 16 } ] },
                { "id": 4, "name": "path", "x": 0, "y": 32,
                  "polyline": [ { "x": 0, "y": 0 }, { "x": 32, "y": 0 } ] },
                { "id": 5, "name": "dot", "point": true, "x": 8, "y": 8 }
            ] }
        ]
    })";

    TileMap const map = LoadTileMap(json);
    REQUIRE(map.GetObjectLayerCount() == 1);
    auto const& layer = map.GetObjectLayer(0);
    REQUIRE(layer.objects.size() == 5);

    auto const& rect = layer.objects[0];
    REQUIRE(rect.kind == ObjectKind::Rectangle);
    REQUIRE(rect.name == "spawn");
    REQUIRE(rect.type == "player");
    REQUIRE(rect.position.x == Catch::Approx(10.5f));
    REQUIRE(rect.position.y == Catch::Approx(20.0f));
    REQUIRE(rect.size.x == Catch::Approx(32.0f));
    REQUIRE(rect.size.y == Catch::Approx(32.0f));
    REQUIRE(rect.rotation == Catch::Approx(45.0f));

    REQUIRE(layer.objects[1].kind == ObjectKind::Ellipse);

    auto const& poly = layer.objects[2];
    REQUIRE(poly.kind == ObjectKind::Polygon);
    REQUIRE(poly.points.size() == 3);

    auto const& line = layer.objects[3];
    REQUIRE(line.kind == ObjectKind::Polyline);
    REQUIRE(line.points.size() == 2);

    REQUIRE(layer.objects[4].kind == ObjectKind::Point);
}

TEST_CASE("Loader: malformed input throws", "[tilemap][loader]") {
    REQUIRE_THROWS(LoadTileMap(R"({"width": 1})"));
    REQUIRE_THROWS(LoadTileMap("not json"));
}

TEST_CASE("Loader: zlib-compressed tile data loads", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 2, "height": 2, "tilewidth": 16, "tileheight": 16,
        "layers": [
            { "type": "tilelayer", "data": "eJxjZGBgYAJiZiBmAWIAAGAACw==", "encoding": "base64", "compression": "zlib" }
        ]
    })";

    TileMap const map = LoadTileMap(json);
    REQUIRE(map.GetTile(0, 0, 0).id == 1);
    REQUIRE(map.GetTile(0, 1, 0).id == 2);
    REQUIRE(map.GetTile(0, 0, 1).id == 3);
    REQUIRE(map.GetTile(0, 1, 1).id == 4);
}

TEST_CASE("Loader: gzip-compressed tile data loads", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 2, "height": 2, "tilewidth": 16, "tileheight": 16,
        "layers": [
            { "type": "tilelayer", "data": "H4sIALwPgWoC/2NkYGBgAmJmIGYBYgDv1AWvEAAAAA==", "encoding": "base64", "compression": "gzip" }
        ]
    })";

    TileMap const map = LoadTileMap(json);
    REQUIRE(map.GetTile(0, 1, 1).id == 4);
}

TEST_CASE("Loader: unsupported compression is rejected", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 1, "height": 1, "tilewidth": 16, "tileheight": 16,
        "layers": [
            { "type": "tilelayer", "data": "AAAA", "encoding": "base64", "compression": "zstd" }
        ]
    })";
    REQUIRE_THROWS(LoadTileMap(json));
}

TEST_CASE("Loader: custom properties load on maps, layers, tilesets, and objects", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 1, "height": 1, "tilewidth": 16, "tileheight": 16,
        "properties": [ { "name": "theme", "type": "string", "value": "forest" } ],
        "tilesets": [
            { "firstgid": 1, "tilewidth": 16, "tileheight": 16, "columns": 4, "tilecount": 4,
              "properties": [ { "name": "author", "type": "string", "value": "me" } ],
              "tiles": [ { "id": 1, "properties": [ { "name": "solid", "type": "bool", "value": true } ] } ]
            }
        ],
        "layers": [
            { "type": "tilelayer", "name": "ground", "data": [2],
              "properties": [ { "name": "z", "type": "int", "value": 3 } ] },
            { "type": "objectgroup", "name": "spawns",
              "properties": [ { "name": "tag", "type": "string", "value": "players" } ],
              "objects": [
                { "id": 1, "x": 0, "y": 0, "width": 16, "height": 16,
                  "properties": [ { "name": "health", "type": "float", "value": 2.5 } ] }
              ] }
        ]
    })";

    TileMap const map = LoadTileMap(json);

    REQUIRE(GetProperty<std::string>(map.properties, "theme") == "forest");
    REQUIRE(HasProperty(map.properties, "theme"));

    REQUIRE(GetProperty<std::string>(map.GetTileset(0).properties, "author") == "me");

    auto const& tileProps = map.GetTileset(0).tileProperties;
    REQUIRE(tileProps.count(1) == 1);
    REQUIRE(GetProperty<bool>(tileProps.at(1), "solid") == true);

    REQUIRE(GetProperty<int>(map.GetLayer(0).properties, "z") == 3);

    REQUIRE(GetProperty<std::string>(map.GetObjectLayer(0).properties, "tag") == "players");
    REQUIRE(GetProperty<float>(map.GetObjectLayer(0).objects[0].properties, "health") == Catch::Approx(2.5f));
}

TEST_CASE("Loader: color properties parse as Color", "[tilemap][loader]") {
    std::string const json = R"({
        "width": 1, "height": 1, "tilewidth": 16, "tileheight": 16,
        "properties": [ { "name": "tint", "type": "color", "value": "#ff0000ff" } ]
    })";

    TileMap const map = LoadTileMap(json);
    auto const tint = GetProperty<moth::core::Color>(map.properties, "tint");
    REQUIRE(tint.r == Catch::Approx(0.0f));
    REQUIRE(tint.g == Catch::Approx(0.0f));
    REQUIRE(tint.b == Catch::Approx(1.0f));
    REQUIRE(tint.a == Catch::Approx(1.0f));
}

TEST_CASE("Properties: GetProperty falls back when absent or wrong type", "[tilemap][loader]") {
    Properties props;
    props["n"] = 5;
    REQUIRE(GetProperty<int>(props, "n") == 5);
    REQUIRE(GetProperty<int>(props, "missing") == 0);
    REQUIRE(GetProperty<int>(props, "missing", 42) == 42);
    REQUIRE(GetProperty<std::string>(props, "n", "fallback") == "fallback");
}

TEST_CASE("Loader: external .tsj tilesets are resolved relative to the map", "[tilemap][loader]") {
    auto const dir = std::filesystem::temp_directory_path() / "moth_tilemap_tsj_test";
    std::filesystem::create_directories(dir);
    {
        std::ofstream tsj(dir / "tiles.tsj");
        tsj << R"({ "name": "tiles", "image": "tiles.png", "imagewidth": 64, "imageheight": 16,
                   "tilewidth": 16, "tileheight": 16, "columns": 4, "tilecount": 4 })";
    }

    std::string const mapJson = R"({
        "width": 1, "height": 1, "tilewidth": 16, "tileheight": 16,
        "tilesets": [ { "firstgid": 1, "source": "tiles.tsj" } ],
        "layers": [ { "type": "tilelayer", "name": "ground", "data": [1] } ]
    })";

    TileMap const map = LoadTileMapFromJson(nlohmann::json::parse(mapJson), dir);
    REQUIRE(map.GetTilesetCount() == 1);
    REQUIRE(map.GetTileset(0).firstGid == 1);
    REQUIRE(map.GetTileset(0).name == "tiles");
    REQUIRE(map.GetTileset(0).columns == 4);
    REQUIRE(map.GetTileset(0).tileCount == 4);
    REQUIRE(map.GetTileset(0).imagePath == "tiles.png");

    std::filesystem::remove_all(dir);
}
