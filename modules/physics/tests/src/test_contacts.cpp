#include "moth/physics/physics.h"

#include <catch2/catch_all.hpp>

using namespace moth::physics;

namespace {
    struct Counter : b2ContactListener {
        int beginCount = 0;
        int endCount = 0;

        void BeginContact(b2Contact*) override { ++beginCount; }
        void EndContact(b2Contact*) override { ++endCount; }
    };

    b2Body* MakeBox(World& world, float x, float y, b2BodyType type) {
        b2BodyDef def;
        def.type = type;
        def.position = b2Vec2{ x, y };
        b2Body* body = world.CreateBody(def);
        b2PolygonShape shape;
        shape.SetAsBox(0.5f, 0.5f);
        b2FixtureDef fd;
        fd.shape = &shape;
        fd.density = (type == b2_dynamicBody) ? 1.0f : 0.0f;
        body->CreateFixture(&fd);
        return body;
    }
}

TEST_CASE("Contacts: BeginContact fires when a body lands on the ground", "[physics][contacts]") {
    World world({ 0.0f, -10.0f });
    Counter counter;
    world.SetContactListener(&counter);

    MakeBox(world, 0.0f, 0.0f, b2_staticBody);      // ground (top at y = 0.5)
    b2Body* body = MakeBox(world, 0.0f, 2.0f, b2_dynamicBody);

    // Step until the body falls and settles on the ground.
    for (int i = 0; i < 240; ++i) {
        world.Step(1.0f / 60.0f);
    }

    REQUIRE(counter.beginCount >= 1);
    // The body should come to rest near the ground's top surface.
    REQUIRE(body->GetPosition().y == Catch::Approx(1.0f).margin(0.1f));
}
