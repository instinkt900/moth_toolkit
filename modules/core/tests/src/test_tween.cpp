#include "moth/core/tween.h"

#include <catch2/catch_all.hpp>

using namespace moth::core;

TEST_CASE("Tween: linear float interpolation", "[core][tween]") {
    Tween<float> t(0.0f, 10.0f, 1.0f);
    REQUIRE(t.GetValue() == Catch::Approx(0.0f));
    t.Update(0.5f);
    REQUIRE(t.GetValue() == Catch::Approx(5.0f));
    t.Update(0.5f);
    REQUIRE(t.IsComplete());
    REQUIRE(t.GetValue() == Catch::Approx(10.0f));
}

TEST_CASE("Tween: easing curves are applied", "[core][tween]") {
    // At t=0.5: QuadIn = 0.25, QuadOut = 0.75.
    Tween<float> in(0.0f, 100.0f, 1.0f, InterpType::QuadIn);
    Tween<float> out(0.0f, 100.0f, 1.0f, InterpType::QuadOut);
    in.Update(0.5f);
    out.Update(0.5f);
    REQUIRE(in.GetValue() == Catch::Approx(25.0f));
    REQUIRE(out.GetValue() == Catch::Approx(75.0f));
}

TEST_CASE("Tween: Vec2 interpolates both components", "[core][tween]") {
    Tween<FloatVec2> t(FloatVec2{ 0.0f, 0.0f }, FloatVec2{ 10.0f, 20.0f }, 1.0f);
    t.Update(0.5f);
    auto const v = t.GetValue();
    REQUIRE(v.x == Catch::Approx(5.0f));
    REQUIRE(v.y == Catch::Approx(10.0f));
}

TEST_CASE("Tween: Color interpolates all components", "[core][tween]") {
    Tween<Color> t(Color{ 0.0f, 0.0f, 0.0f, 0.0f }, Color{ 1.0f, 1.0f, 1.0f, 1.0f }, 1.0f);
    t.Update(0.5f);
    auto const c = t.GetValue();
    REQUIRE(c.r == Catch::Approx(0.5f));
    REQUIRE(c.g == Catch::Approx(0.5f));
    REQUIRE(c.b == Catch::Approx(0.5f));
    REQUIRE(c.a == Catch::Approx(0.5f));
}

TEST_CASE("Tween: overshoot clamps to the end value", "[core][tween]") {
    Tween<float> t(0.0f, 1.0f, 0.5f);
    t.Update(10.0f);
    REQUIRE(t.IsComplete());
    REQUIRE(t.Progress() == 1.0f);
    REQUIRE(t.GetValue() == Catch::Approx(1.0f));
}

TEST_CASE("Tween: Restart replays from the beginning", "[core][tween]") {
    Tween<float> t(0.0f, 1.0f, 1.0f);
    t.Update(1.0f);
    REQUIRE(t.IsComplete());
    t.Restart();
    REQUIRE_FALSE(t.IsComplete());
    REQUIRE(t.GetValue() == Catch::Approx(0.0f));
}

TEST_CASE("Tween: zero duration is complete immediately", "[core][tween]") {
    Tween<float> t(0.0f, 5.0f, 0.0f);
    REQUIRE(t.IsComplete());
    REQUIRE(t.GetValue() == Catch::Approx(5.0f));
}
