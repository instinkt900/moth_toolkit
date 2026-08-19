#include "moth/tilemap/tile_map_collision.h"
#include "moth/tilemap/tile_map_loader.h"
#include "moth/tilemap/tile_map_physics.h"

#include <catch2/catch_all.hpp>

#include <vector>

using namespace moth::tilemap;

namespace {
    std::string CollisionMapJson() {
        return R"({
            "width": 2, "height": 1, "tilewidth": 16, "tileheight": 16,
            "tilesets": [
                { "firstgid": 1, "name": "ts", "image": "ts.png",
                  "imagewidth": 32, "imageheight": 16, "tilewidth": 16, "tileheight": 16,
                  "columns": 2, "tilecount": 2,
                  "tiles": [
                      { "id": 0, "objectgroup": { "objects": [
                          { "id": 1, "x": 2, "y": 2, "width": 12, "height": 12 },
                          { "id": 2, "x": 0, "y": 0,
                            "polygon": [ { "x": 0, "y": 0 }, { "x": 16, "y": 0 }, { "x": 16, "y": 16 } ] }
                      ] } }
                  ] }
            ],
            "layers": [ { "type": "tilelayer", "name": "ground", "data": [1, 1] } ]
        })";
    }
}

TEST_CASE("Loader: tile collision objectgroup parses", "[tilemap][collision]") {
    TileMap const map = LoadTileMap(CollisionMapJson());
    auto const& tileset = map.GetTileset(0);

    REQUIRE(tileset.tileCollisions.count(0) == 1);
    auto const& shapes = tileset.tileCollisions.at(0);
    REQUIRE(shapes.size() == 2);

    REQUIRE(shapes[0].kind == ObjectKind::Rectangle);
    REQUIRE(shapes[0].position.x == Catch::Approx(2.0f));
    REQUIRE(shapes[0].position.y == Catch::Approx(2.0f));
    REQUIRE(shapes[0].size.x == Catch::Approx(12.0f));
    REQUIRE(shapes[0].size.y == Catch::Approx(12.0f));

    REQUIRE(shapes[1].kind == ObjectKind::Polygon);
    REQUIRE(shapes[1].points.size() == 3);
}

TEST_CASE("Collision: CollectLayerCollisions offsets shapes to world space", "[tilemap][collision]") {
    TileMap const map = LoadTileMap(CollisionMapJson());

    auto const shapes = CollectLayerCollisions(map, 0);
    REQUIRE(shapes.size() == 4); // two tiles, two shapes each

    // Tile (0, 0): rectangle at world (2, 2).
    REQUIRE(shapes[0].position.x == Catch::Approx(2.0f));
    REQUIRE(shapes[0].position.y == Catch::Approx(2.0f));

    // Tile (1, 0): rectangle offset by the tile width -> (18, 2).
    REQUIRE(shapes[2].position.x == Catch::Approx(18.0f));
    REQUIRE(shapes[2].position.y == Catch::Approx(2.0f));

    // Out-of-range layer yields nothing.
    REQUIRE(CollectLayerCollisions(map, 1).empty());
}

TEST_CASE("Collision: CollectLayerCollisions skips empty tiles and shapes", "[tilemap][collision]") {
    std::string const json = R"({
        "width": 2, "height": 1, "tilewidth": 16, "tileheight": 16,
        "tilesets": [
            { "firstgid": 1, "name": "ts", "image": "ts.png",
              "imagewidth": 32, "imageheight": 16, "tilewidth": 16, "tileheight": 16,
              "columns": 2, "tilecount": 2,
              "tiles": [
                  { "id": 0, "objectgroup": { "objects": [
                      { "id": 1, "x": 0, "y": 0, "width": 16, "height": 16 }
                  ] } }
              ] }
        ],
        "layers": [ { "type": "tilelayer", "name": "ground", "data": [0, 1] } ]
    })";

    TileMap const map = LoadTileMap(json);
    auto const shapes = CollectLayerCollisions(map, 0);
    REQUIRE(shapes.size() == 1); // empty tile (0, 0) contributes nothing
    REQUIRE(shapes[0].position.x == Catch::Approx(16.0f));
}

TEST_CASE("Physics: builds a static body from collision shapes", "[tilemap][collision]") {
    std::vector<MapObject> shapes;

    MapObject rect;
    rect.kind = ObjectKind::Rectangle;
    rect.position = { 0.0f, 0.0f };
    rect.size = { 16.0f, 16.0f };
    shapes.push_back(rect);

    MapObject circle;
    circle.kind = ObjectKind::Ellipse;
    circle.position = { 16.0f, 0.0f };
    circle.size = { 8.0f, 8.0f };
    shapes.push_back(circle);

    MapObject point;
    point.kind = ObjectKind::Point;
    shapes.push_back(point); // ignored

    b2World world(b2Vec2{ 0.0f, -10.0f });
    b2Body* body = CreateStaticCollisionBody(world, shapes);

    REQUIRE(body != nullptr);
    REQUIRE(body->GetType() == b2_staticBody);

    int fixtureCount = 0;
    for (b2Fixture* fixture = body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext()) {
        ++fixtureCount;
    }
    REQUIRE(fixtureCount == 2); // point object skipped
}
