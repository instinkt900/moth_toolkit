#include "moth/physics/physics.h"

#include <catch2/catch_all.hpp>

using namespace moth::physics;

namespace {
    b2Body* MakeBox(World& world, float x, float y, float halfExtent = 0.5f) {
        b2BodyDef def;
        def.type = b2_dynamicBody;
        def.position = b2Vec2{ x, y };
        b2Body* body = world.CreateBody(def);
        b2PolygonShape shape;
        shape.SetAsBox(halfExtent, halfExtent);
        b2FixtureDef fd;
        fd.shape = &shape;
        fd.density = 1.0f;
        body->CreateFixture(&fd);
        return body;
    }
}

TEST_CASE("Body: a dynamic body falls under gravity", "[physics][body]") {
    World world({ 0.0f, -10.0f });
    b2Body* body = MakeBox(world, 0.0f, 10.0f);
    float const startY = body->GetPosition().y;

    for (int i = 0; i < 60; ++i) {
        world.Step(1.0f / 60.0f);
    }

    REQUIRE(body->GetPosition().y < startY);
}

TEST_CASE("Body: a body with zero gravity stays put", "[physics][body]") {
    World world({ 0.0f, 0.0f });
    b2Body* body = MakeBox(world, 0.0f, 0.0f);

    for (int i = 0; i < 60; ++i) {
        world.Step(1.0f / 60.0f);
    }

    REQUIRE(body->GetPosition().y == Catch::Approx(0.0f).margin(0.001f));
}

TEST_CASE("Body: ApplyForceToCenter accelerates a body", "[physics][body]") {
    World world({ 0.0f, 0.0f });
    b2Body* body = MakeBox(world, 0.0f, 0.0f);

    body->SetLinearVelocity(b2Vec2{ 0.0f, 0.0f });
    for (int i = 0; i < 30; ++i) {
        body->ApplyForceToCenter(b2Vec2{ 10.0f, 0.0f }, true);
        world.Step(1.0f / 60.0f);
    }

    REQUIRE(body->GetLinearVelocity().x > 0.0f);
}

TEST_CASE("Body: fixture mass comes from density", "[physics][body]") {
    World world;
    b2Body* body = MakeBox(world, 0.0f, 0.0f);
    // A 1x1 box (half-extent 0.5) at density 1.0 has area 1.0 -> mass 1.0.
    REQUIRE(body->GetMass() == Catch::Approx(1.0f).margin(0.001f));
}
