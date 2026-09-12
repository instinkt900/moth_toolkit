#include <catch2/catch_test_macros.hpp>

#include <moth/core/angle.h>

#include <cmath>

using namespace moth::core;

namespace {
    bool Near(float a, float b, float eps = 1e-4f) {
        return std::fabs(a - b) < eps;
    }
}

TEST_CASE("DegToRad and RadToDeg convert between units", "[angle]") {
    CHECK(Near(DegToRad(0.0f), 0.0f));
    CHECK(Near(DegToRad(180.0f), kPi));
    CHECK(Near(DegToRad(-90.0f), -kPi / 2.0f));
    CHECK(Near(RadToDeg(kPi), 180.0f));
    CHECK(Near(RadToDeg(DegToRad(37.5f)), 37.5f));
}

TEST_CASE("WrapAngleRadians wraps to [-pi, pi]", "[angle]") {
    CHECK(Near(WrapAngleRadians(0.0f), 0.0f));
    CHECK(Near(WrapAngleRadians(kPi), -kPi));
    CHECK(Near(WrapAngleRadians(-kPi), -kPi));
    CHECK(Near(WrapAngleRadians(kPi * 1.5f), -kPi * 0.5f));
    CHECK(Near(WrapAngleRadians(kTwoPi * 2.0f), 0.0f));
}

TEST_CASE("AngleDeltaRadians takes the shortest path", "[angle]") {
    CHECK(Near(AngleDeltaRadians(0.0f, kPi / 2.0f), kPi / 2.0f));
    CHECK(Near(AngleDeltaRadians(DegToRad(170.0f), DegToRad(-170.0f)), DegToRad(20.0f))); // across the +-pi seam
    CHECK(Near(AngleDeltaRadians(DegToRad(-170.0f), DegToRad(170.0f)), DegToRad(-20.0f)));
}

TEST_CASE("LerpAngleRadians interpolates along the shortest arc", "[angle]") {
    // From 170 to -170 degrees is +20 degrees (not -340): halfway is 180.
    CHECK(Near(LerpAngleRadians(DegToRad(170.0f), DegToRad(-170.0f), 0.5f), kPi));
    // Simple in-range case.
    CHECK(Near(LerpAngleRadians(0.0f, kPi / 2.0f, 0.5f), kPi / 4.0f));
}
