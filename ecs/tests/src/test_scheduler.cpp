#include "moth/ecs/ecs.h"

#include <catch2/catch_all.hpp>

using namespace moth::ecs;

namespace {
    struct Count {
        int value = 0;
    };
}

TEST_CASE("Scheduler: runs systems in registration order", "[ecs][scheduler]") {
    World world;
    auto const entity = world.Create();
    world.Emplace<Count>(entity);

    Scheduler scheduler;
    std::vector<int> order;
    scheduler.Add([&](World&, float) { order.push_back(1); });
    scheduler.Add([&](World&, float) { order.push_back(2); });
    scheduler.Add([&](World&, float) { order.push_back(3); });

    scheduler.Run(world, 0.016f);

    REQUIRE(order == std::vector<int>{ 1, 2, 3 });
}

TEST_CASE("Scheduler: passes dt to every system", "[ecs][scheduler]") {
    World world;
    float total = 0.0f;

    Scheduler scheduler;
    scheduler.Add([&](World&, float dt) { total += dt; });
    scheduler.Add([&](World&, float dt) { total += dt; });

    scheduler.Run(world, 0.5f);
    REQUIRE(total == 1.0f);
}

TEST_CASE("Scheduler: systems can mutate the world", "[ecs][scheduler]") {
    World world;
    auto const entity = world.Create();
    world.Emplace<Count>(entity);

    Scheduler scheduler;
    scheduler.Add([](World& w, float) {
        w.Each<Count>([](Count& count) { count.value += 1; });
    });
    scheduler.Add([](World& w, float) {
        w.Each<Count>([](Count& count) { count.value += 1; });
    });

    scheduler.Run(world, 0.016f);
    REQUIRE(world.Get<Count>(entity).value == 2);
}

TEST_CASE("Scheduler: Size and Clear", "[ecs][scheduler]") {
    Scheduler scheduler;
    REQUIRE(scheduler.Size() == 0);

    scheduler.Add([](World&, float) {});
    scheduler.Add([](World&, float) {});
    REQUIRE(scheduler.Size() == 2);

    scheduler.Clear();
    REQUIRE(scheduler.Size() == 0);
}
