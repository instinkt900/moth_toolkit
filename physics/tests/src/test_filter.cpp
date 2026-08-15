#include "moth/physics/physics.h"

#include <catch2/catch_all.hpp>

using namespace moth::physics;

namespace {
    struct Counter : b2ContactListener {
        int beginCount = 0;
        void BeginContact(b2Contact*) override { ++beginCount; }
    };

    b2Body* MakeBox(World& world, float x, float y, b2Filter filter) {
        b2BodyDef def;
        def.type = b2_dynamicBody;
        def.position = b2Vec2{ x, y };
        b2Body* body = world.CreateBody(def);
        b2PolygonShape shape;
        shape.SetAsBox(0.5f, 0.5f);
        b2FixtureDef fd;
        fd.shape = &shape;
        fd.density = 1.0f;
        fd.filter = filter;
        body->CreateFixture(&fd);
        return body;
    }
}

TEST_CASE("Filter: disjoint category/mask bits prevent collision", "[physics][filter]") {
    World world({ 0.0f, 0.0f });  // no gravity, so overlap is purely from placement
    Counter counter;
    world.SetContactListener(&counter);

    b2Filter a;
    a.categoryBits = 0x0001;
    a.maskBits = 0x0001;

    b2Filter b;
    b.categoryBits = 0x0002;
    b.maskBits = 0x0002;

    MakeBox(world, 0.0f, 0.0f, a);
    MakeBox(world, 0.1f, 0.0f, b);  // overlapping but filtered out

    for (int i = 0; i < 10; ++i) {
        world.Step(1.0f / 60.0f);
    }

    REQUIRE(counter.beginCount == 0);
}

TEST_CASE("Filter: matching category/mask bits allow collision", "[physics][filter]") {
    World world({ 0.0f, 0.0f });
    Counter counter;
    world.SetContactListener(&counter);

    b2Filter a;
    a.categoryBits = 0x0001;
    a.maskBits = 0x0001;

    b2Filter b;
    b.categoryBits = 0x0001;
    b.maskBits = 0x0001;

    MakeBox(world, 0.0f, 0.0f, a);
    MakeBox(world, 0.1f, 0.0f, b);  // overlapping and allowed to collide

    for (int i = 0; i < 10; ++i) {
        world.Step(1.0f / 60.0f);
    }

    REQUIRE(counter.beginCount >= 1);
}
