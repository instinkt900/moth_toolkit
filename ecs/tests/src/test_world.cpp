#include "moth/ecs/ecs.h"

#include <catch2/catch_all.hpp>

using namespace moth::ecs;

namespace {
    struct Position {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct Velocity {
        float x = 0.0f;
        float y = 0.0f;
    };
}

TEST_CASE("World: Create returns a valid entity", "[ecs][world]") {
    World world;
    auto const entity = world.Create();
    REQUIRE(entity != kNullEntity);
    REQUIRE(world.Valid(entity));
    REQUIRE(world.Size() == 1);
}

TEST_CASE("World: Destroy invalidates an entity", "[ecs][world]") {
    World world;
    auto const entity = world.Create();
    world.Destroy(entity);
    REQUIRE_FALSE(world.Valid(entity));
    REQUIRE(world.Size() == 0);
}

TEST_CASE("World: Clear destroys all entities", "[ecs][world]") {
    World world;
    world.Create();
    world.Create();
    REQUIRE(world.Size() == 2);
    world.Clear();
    REQUIRE(world.Size() == 0);
}

TEST_CASE("World: Emplace, Get, and Has manage components", "[ecs][world]") {
    World world;
    auto const entity = world.Create();

    REQUIRE_FALSE(world.Has<Position>(entity));
    world.Emplace<Position>(entity, 1.0f, 2.0f);
    REQUIRE(world.Has<Position>(entity));

    auto const& position = world.Get<Position>(entity);
    REQUIRE(position.x == 1.0f);
    REQUIRE(position.y == 2.0f);

    world.Remove<Position>(entity);
    REQUIRE_FALSE(world.Has<Position>(entity));
}

TEST_CASE("World: Each visits entities with all components", "[ecs][world]") {
    World world;

    auto const a = world.Create();
    world.Emplace<Position>(a);
    world.Emplace<Velocity>(a);

    auto const b = world.Create();
    world.Emplace<Position>(b);

    int visited = 0;
    world.Each<Position, Velocity>([&](Entity, Position const&, Velocity const&) {
        ++visited;
    });
    REQUIRE(visited == 1);  // only entity a has both
}

TEST_CASE("World: View supports range-based iteration", "[ecs][world]") {
    World world;

    auto const a = world.Create();
    world.Emplace<Position>(a, 10.0f, 20.0f);

    float sumX = 0.0f;
    int count = 0;
    // A single-component view iterates entities directly (EnTT semantics); a
    // multi-component view yields (entity, component...) tuples.
    for (auto entity : world.View<Position>()) {
        sumX += world.Get<Position>(entity).x;
        ++count;
    }
    REQUIRE(count == 1);
    REQUIRE(sumX == 10.0f);
}

TEST_CASE("World: Raw exposes the underlying registry", "[ecs][world]") {
    World world;
    auto const entity = world.Raw().create();
    REQUIRE(world.Valid(entity));
}
