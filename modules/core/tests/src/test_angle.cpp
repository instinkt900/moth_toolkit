#include <catch2/catch_test_macros.hpp>

#include <moth/core/angle.h>

#include <cmath>

using namespace moth::core;

namespace {
    bool Near(float a, float b, float eps = 1e-4f) {
        return std::fabs(a - b) < eps;
    }
}

TEST_CASE("WrapAngleDegrees wraps to [-180, 180]", "[angle]") {
    CHECK(Near(WrapAngleDegrees(0.0f), 0.0f));
    CHECK(Near(WrapAngleDegrees(180.0f), -180.0f));
    CHECK(Near(WrapAngleDegrees(-180.0f), -180.0f));
    CHECK(Near(WrapAngleDegrees(200.0f), -160.0f));
    CHECK(Near(WrapAngleDegrees(-200.0f), 160.0f));
    CHECK(Near(WrapAngleDegrees(720.0f), 0.0f));
}

TEST_CASE("WrapAngleRadians wraps to [-pi, pi]", "[angle]") {
    CHECK(Near(WrapAngleRadians(0.0f), 0.0f));
    CHECK(Near(WrapAngleRadians(kPi), -kPi));
    CHECK(Near(WrapAngleRadians(-kPi), -kPi));
    CHECK(Near(WrapAngleRadians(kPi * 1.5f), -kPi * 0.5f));
}

TEST_CASE("AngleDeltaDegrees takes the shortest path", "[angle]") {
    CHECK(Near(AngleDeltaDegrees(0.0f, 90.0f), 90.0f));
    CHECK(Near(AngleDeltaDegrees(170.0f, -170.0f), 20.0f)); // across the +-180 seam
    CHECK(Near(AngleDeltaDegrees(-170.0f, 170.0f), -20.0f));
}

TEST_CASE("LerpAngleDegrees interpolates along the shortest arc", "[angle]") {
    // From 170 to -170 is +20 degrees (not -340): halfway is 180.
    CHECK(Near(LerpAngleDegrees(170.0f, -170.0f, 0.5f), 180.0f));
    // Simple in-range case.
    CHECK(Near(LerpAngleDegrees(0.0f, 90.0f, 0.5f), 45.0f));
}

TEST_CASE("AngleDeltaRadians matches degrees", "[angle]") {
    CHECK(Near(AngleDeltaRadians(0.0f, kPi / 2.0f), kPi / 2.0f));
}
