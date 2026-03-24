#include "moth_ui/utils/transform.h"
#include <catch2/catch_all.hpp>
#include <cmath>

using namespace moth_ui;

static constexpr float kPi = 3.14159265358979f;

TEST_CASE("FloatMat4x4 Identity leaves point unchanged", "[transform]") {
    auto const t = FloatMat4x4::Identity();
    auto const p = t.TransformPoint({ 3.0f, 7.0f });
    REQUIRE(p.x == Catch::Approx(3.0f));
    REQUIRE(p.y == Catch::Approx(7.0f));
}

TEST_CASE("FloatMat4x4 Translation moves a point", "[transform]") {
    auto const t = FloatMat4x4::Translation({ 10.0f, -5.0f });
    auto const p = t.TransformPoint({ 1.0f, 2.0f });
    REQUIRE(p.x == Catch::Approx(11.0f));
    REQUIRE(p.y == Catch::Approx(-3.0f));
}

TEST_CASE("FloatMat4x4 Rotation 90 degrees around origin", "[transform]") {
    auto const t = FloatMat4x4::Rotation(90.0f, { 0.0f, 0.0f });
    auto const p = t.TransformPoint({ 1.0f, 0.0f });
    // 90 degree CW rotation: (1,0) -> (0,1)
    REQUIRE(p.x == Catch::Approx(0.0f).margin(1e-5f));
    REQUIRE(p.y == Catch::Approx(1.0f).margin(1e-5f));
}

TEST_CASE("FloatMat4x4 Rotation 180 degrees around origin", "[transform]") {
    auto const t = FloatMat4x4::Rotation(180.0f, { 0.0f, 0.0f });
    auto const p = t.TransformPoint({ 5.0f, 3.0f });
    REQUIRE(p.x == Catch::Approx(-5.0f).margin(1e-5f));
    REQUIRE(p.y == Catch::Approx(-3.0f).margin(1e-5f));
}

TEST_CASE("FloatMat4x4 Rotation 0 degrees leaves point unchanged", "[transform]") {
    auto const t = FloatMat4x4::Rotation(0.0f, { 50.0f, 50.0f });
    auto const p = t.TransformPoint({ 20.0f, 30.0f });
    REQUIRE(p.x == Catch::Approx(20.0f));
    REQUIRE(p.y == Catch::Approx(30.0f));
}

TEST_CASE("FloatMat4x4 Rotation around non-origin pivot", "[transform]") {
    // 90 degree CW rotation around (1,1): (1,0) -> (2,1)
    auto const t = FloatMat4x4::Rotation(90.0f, { 1.0f, 1.0f });
    auto const p = t.TransformPoint({ 1.0f, 0.0f });
    REQUIRE(p.x == Catch::Approx(2.0f).margin(1e-5f));
    REQUIRE(p.y == Catch::Approx(1.0f).margin(1e-5f));
}

TEST_CASE("FloatMat4x4 composition applies rhs first", "[transform]") {
    // Translate then rotate: translate (1,0) by (5,0) = (6,0), then rotate 90 CW = (0,6)
    auto const translate = FloatMat4x4::Translation({ 5.0f, 0.0f });
    auto const rotate    = FloatMat4x4::Rotation(90.0f, { 0.0f, 0.0f });
    auto const combined  = rotate * translate; // rhs (translate) applied first
    auto const p = combined.TransformPoint({ 1.0f, 0.0f });
    REQUIRE(p.x == Catch::Approx(0.0f).margin(1e-5f));
    REQUIRE(p.y == Catch::Approx(6.0f).margin(1e-5f));
}

TEST_CASE("FloatMat4x4 GetRotationDegrees round-trips", "[transform]") {
    for (float deg : { 0.0f, 45.0f, 90.0f, 135.0f, -45.0f, -90.0f }) {
        auto const t = FloatMat4x4::Rotation(deg, { 0.0f, 0.0f });
        REQUIRE(t.GetRotationDegrees() == Catch::Approx(deg).margin(1e-4f));
    }
}

TEST_CASE("FloatMat4x4 Invert undoes translation", "[transform]") {
    auto const t = FloatMat4x4::Translation({ 7.0f, -3.0f });
    auto const inv = t.Invert();
    auto const p = inv.TransformPoint({ 7.0f, -3.0f });
    REQUIRE(p.x == Catch::Approx(0.0f).margin(1e-5f));
    REQUIRE(p.y == Catch::Approx(0.0f).margin(1e-5f));
}

TEST_CASE("FloatMat4x4 Invert undoes rotation", "[transform]") {
    auto const t   = FloatMat4x4::Rotation(45.0f, { 0.0f, 0.0f });
    auto const inv = t.Invert();
    FloatVec2 const original{ 3.0f, 1.0f };
    auto const roundTrip = inv.TransformPoint(t.TransformPoint(original));
    REQUIRE(roundTrip.x == Catch::Approx(original.x).margin(1e-5f));
    REQUIRE(roundTrip.y == Catch::Approx(original.y).margin(1e-5f));
}

TEST_CASE("FloatMat4x4 Invert undoes Translation * Rotation", "[transform]") {
    auto const t   = FloatMat4x4::Translation({ 10.0f, 5.0f }) * FloatMat4x4::Rotation(30.0f, { 5.0f, 5.0f });
    auto const inv = t.Invert();
    FloatVec2 const original{ 8.0f, 2.0f };
    auto const roundTrip = inv.TransformPoint(t.TransformPoint(original));
    REQUIRE(roundTrip.x == Catch::Approx(original.x).margin(1e-4f));
    REQUIRE(roundTrip.y == Catch::Approx(original.y).margin(1e-4f));
}
