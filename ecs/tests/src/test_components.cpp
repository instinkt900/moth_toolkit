#include "moth/ecs/ecs.h"

#include <catch2/catch_all.hpp>

using namespace moth::ecs;

TEST_CASE("Transform component defaults to identity", "[ecs][components]") {
    Transform transform;
    REQUIRE(transform.transform.position.x == 0.0f);
    REQUIRE(transform.transform.position.y == 0.0f);
    REQUIRE(transform.transform.rotation == 0.0f);
    REQUIRE(transform.transform.scale.x == 1.0f);
    REQUIRE(transform.transform.scale.y == 1.0f);
}

TEST_CASE("Active component defaults to true", "[ecs][components]") {
    Active active;
    REQUIRE(active.value == true);
}

TEST_CASE("Tag component stores a name", "[ecs][components]") {
    Tag tag{ "player" };
    REQUIRE(tag.name == "player");
}

TEST_CASE("Components live independently per entity", "[ecs][components]") {
    World world;

    auto const a = world.Create();
    world.Emplace<Transform>(a);
    world.Emplace<Tag>(a, "a");

    auto const b = world.Create();
    world.Emplace<Transform>(b);
    world.Emplace<Tag>(b, "b");

    world.Get<Transform>(a).transform.position.x = 5.0f;
    world.Get<Transform>(b).transform.position.x = -5.0f;

    REQUIRE(world.Get<Tag>(a).name == "a");
    REQUIRE(world.Get<Tag>(b).name == "b");
    REQUIRE(world.Get<Transform>(a).transform.position.x == 5.0f);
    REQUIRE(world.Get<Transform>(b).transform.position.x == -5.0f);
}
