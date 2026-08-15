#include "moth/physics/physics.h"

#include <catch2/catch_all.hpp>

using namespace moth::physics;

TEST_CASE("World: default gravity is 0,-10", "[physics][world]") {
    World world;
    auto const g = world.GetGravity();
    REQUIRE(g.x == Catch::Approx(0.0f));
    REQUIRE(g.y == Catch::Approx(-10.0f));
}

TEST_CASE("World: SetGravity updates gravity", "[physics][world]") {
    World world({ 0.0f, 0.0f });
    world.SetGravity({ 0.0f, -5.0f });
    auto const g = world.GetGravity();
    REQUIRE(g.y == Catch::Approx(-5.0f));
}

TEST_CASE("World: CreateBody returns a live body", "[physics][world]") {
    World world;
    b2BodyDef def;
    b2Body* body = world.CreateBody(def);
    REQUIRE(body != nullptr);
}

TEST_CASE("World: move construction preserves the world", "[physics][world]") {
    World a({ 0.0f, -3.0f });
    World b(std::move(a));
    auto const g = b.GetGravity();
    REQUIRE(g.y == Catch::Approx(-3.0f));
}

TEST_CASE("World: Raw exposes the underlying Box2D world", "[physics][world]") {
    World world;
    REQUIRE(world.Raw().GetGravity().y == Catch::Approx(-10.0f));
}

TEST_CASE("Vec: ToB2 and FromB2 round-trip", "[physics][world]") {
    FloatVec2 const v{ 3.5f, -7.25f };
    auto const roundTripped = FromB2(ToB2(v));
    REQUIRE(roundTripped.x == Catch::Approx(v.x));
    REQUIRE(roundTripped.y == Catch::Approx(v.y));
}
