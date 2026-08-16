#include "moth/core/timer.h"

#include <catch2/catch_all.hpp>

using namespace moth::core;

TEST_CASE("Timer: completes after its duration", "[core][timer]") {
    Timer t(1.0f);
    REQUIRE_FALSE(t.IsComplete());
    t.Update(0.5f);
    REQUIRE_FALSE(t.IsComplete());
    t.Update(0.5f);
    REQUIRE(t.IsComplete());
}

TEST_CASE("Timer: Progress clamps to [0, 1] on overshoot", "[core][timer]") {
    Timer t(1.0f);
    t.Update(5.0f);
    REQUIRE(t.IsComplete());
    REQUIRE(t.Progress() == 1.0f);
    REQUIRE(t.GetRemaining() == 0.0f);
}

TEST_CASE("Timer: Reset restarts the same duration", "[core][timer]") {
    Timer t(1.0f);
    t.Update(1.0f);
    REQUIRE(t.IsComplete());
    t.Reset();
    REQUIRE_FALSE(t.IsComplete());
    REQUIRE(t.Progress() == 0.0f);
}

TEST_CASE("Timer: Start re-arms with a new duration", "[core][timer]") {
    Timer t;
    t.Start(0.5f);
    REQUIRE(t.GetDuration() == 0.5f);
    t.Update(0.5f);
    REQUIRE(t.IsComplete());

    t.Start(1.0f);
    REQUIRE_FALSE(t.IsComplete());
    REQUIRE(t.GetDuration() == 1.0f);
    REQUIRE(t.GetElapsed() == 0.0f);
}

TEST_CASE("Timer: GetRemaining counts down", "[core][timer]") {
    Timer t(2.0f);
    REQUIRE(t.GetRemaining() == Catch::Approx(2.0f));
    t.Update(0.5f);
    REQUIRE(t.GetRemaining() == Catch::Approx(1.5f));
    t.Update(10.0f);
    REQUIRE(t.GetRemaining() == 0.0f);
}

TEST_CASE("Stopwatch: measures elapsed while running", "[core][timer]") {
    Stopwatch s;
    s.Start();
    s.Update(0.1f);
    s.Update(0.2f);
    REQUIRE(s.GetElapsed() == Catch::Approx(0.3f));
}

TEST_CASE("Stopwatch: Stop pauses and Start resumes", "[core][timer]") {
    Stopwatch s;
    s.Start();
    s.Update(0.1f);
    s.Stop();
    s.Update(0.5f);  // ignored while stopped
    REQUIRE(s.GetElapsed() == Catch::Approx(0.1f));
    s.Start();
    s.Update(0.1f);
    REQUIRE(s.GetElapsed() == Catch::Approx(0.2f));
}

TEST_CASE("Stopwatch: Restart zeroes and runs; Reset zeroes and stops", "[core][timer]") {
    Stopwatch s;
    s.Start();
    s.Update(1.0f);
    s.Restart();
    REQUIRE(s.IsRunning());
    REQUIRE(s.GetElapsed() == 0.0f);

    s.Reset();
    REQUIRE_FALSE(s.IsRunning());
    REQUIRE(s.GetElapsed() == 0.0f);
}

TEST_CASE("Cooldown: ready immediately, cools down after firing", "[core][timer]") {
    Cooldown c(1.0f);
    REQUIRE(c.IsReady());
    REQUIRE(c.TryFire());        // fires and starts the cooldown
    REQUIRE_FALSE(c.IsReady());
    REQUIRE_FALSE(c.TryFire());  // still cooling

    c.Update(1.0f);
    REQUIRE(c.IsReady());
    REQUIRE(c.TryFire());
}

TEST_CASE("Cooldown: Reset makes it ready immediately", "[core][timer]") {
    Cooldown c(1.0f);
    c.TryFire();
    REQUIRE_FALSE(c.IsReady());
    c.Reset();
    REQUIRE(c.IsReady());
}
