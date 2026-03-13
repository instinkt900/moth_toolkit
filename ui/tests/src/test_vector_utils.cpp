#include "moth_ui/utils/vector_utils.h"
#include "string_helpers.h"
#include <catch2/catch_all.hpp>

#include <cmath>

using namespace moth_ui;

TEST_CASE("Dot product of orthogonal vectors is zero", "[vector_utils][dot]") {
    FloatVec2 right{ 1.0f, 0.0f };
    FloatVec2 up{ 0.0f, 1.0f };
    REQUIRE(Dot(right, up) == Catch::Approx(0.0f));
}

TEST_CASE("Dot product of parallel vectors", "[vector_utils][dot]") {
    FloatVec2 a{ 3.0f, 4.0f };
    FloatVec2 b{ 6.0f, 8.0f };
    REQUIRE(Dot(a, b) == Catch::Approx(50.0f));
}

TEST_CASE("Dot product 3D", "[vector_utils][dot]") {
    Vector<float, 3> a{ 1.0f, 2.0f, 3.0f };
    Vector<float, 3> b{ 4.0f, 5.0f, 6.0f };
    REQUIRE(Dot(a, b) == Catch::Approx(32.0f));
}

TEST_CASE("LengthSq", "[vector_utils][length]") {
    FloatVec2 v{ 3.0f, 4.0f };
    REQUIRE(LengthSq(v) == Catch::Approx(25.0f));
}

TEST_CASE("Length of 3-4-5 triangle", "[vector_utils][length]") {
    FloatVec2 v{ 3.0f, 4.0f };
    REQUIRE(Length(v) == Catch::Approx(5.0f));
}

TEST_CASE("Length of unit vector", "[vector_utils][length]") {
    FloatVec2 v{ 1.0f, 0.0f };
    REQUIRE(Length(v) == Catch::Approx(1.0f));
}

TEST_CASE("DistanceSq between two points", "[vector_utils][distance]") {
    FloatVec2 a{ 0.0f, 0.0f };
    FloatVec2 b{ 3.0f, 4.0f };
    REQUIRE(DistanceSq(a, b) == Catch::Approx(25.0f));
    REQUIRE(DistanceSq(b, a) == Catch::Approx(25.0f));
}

TEST_CASE("Distance between two points", "[vector_utils][distance]") {
    FloatVec2 a{ 1.0f, 1.0f };
    FloatVec2 b{ 4.0f, 5.0f };
    REQUIRE(Distance(a, b) == Catch::Approx(5.0f));
}

TEST_CASE("Distance from point to itself is zero", "[vector_utils][distance]") {
    FloatVec2 a{ 3.0f, 7.0f };
    REQUIRE(Distance(a, a) == Catch::Approx(0.0f));
}

TEST_CASE("Normalized vector has unit length", "[vector_utils][normalized]") {
    FloatVec2 v{ 3.0f, 4.0f };
    auto n = Normalized(v);
    REQUIRE(Length(n) == Catch::Approx(1.0f).epsilon(0.001));
}

TEST_CASE("Normalized preserves direction", "[vector_utils][normalized]") {
    FloatVec2 v{ 3.0f, 4.0f };
    auto n = Normalized(v);
    REQUIRE(n.x == Catch::Approx(0.6f));
    REQUIRE(n.y == Catch::Approx(0.8f));
}

TEST_CASE("Rotate2D by 90 degrees", "[vector_utils][rotate]") {
    float const half_pi = 1.5707963267948966f;
    FloatVec2 v{ 1.0f, 0.0f };
    auto r = Rotate2D(v, half_pi);
    REQUIRE(r.x == Catch::Approx(0.0f).margin(1e-6f));
    REQUIRE(r.y == Catch::Approx(1.0f).epsilon(0.001));
}

TEST_CASE("Rotate2D by 180 degrees", "[vector_utils][rotate]") {
    float const pi = 3.14159265358979f;
    FloatVec2 v{ 1.0f, 0.0f };
    auto r = Rotate2D(v, pi);
    REQUIRE(r.x == Catch::Approx(-1.0f).epsilon(0.001));
    REQUIRE(r.y == Catch::Approx(0.0f).margin(1e-6f));
}

TEST_CASE("Rotate2D by 0 degrees returns original vector", "[vector_utils][rotate]") {
    FloatVec2 v{ 3.0f, 4.0f };
    auto r = Rotate2D(v, 0.0f);
    REQUIRE(r.x == Catch::Approx(3.0f));
    REQUIRE(r.y == Catch::Approx(4.0f));
}

TEST_CASE("Translate adds vectors", "[vector_utils][translate]") {
    FloatVec2 v{ 1.0f, 2.0f };
    FloatVec2 t{ 3.0f, 4.0f };
    auto result = Translate(v, t);
    REQUIRE(result.x == 4.0f);
    REQUIRE(result.y == 6.0f);
}

TEST_CASE("Angle between same direction vectors is zero", "[vector_utils][angle]") {
    FloatVec2 v{ 1.0f, 0.0f };
    REQUIRE(Angle(v, v) == Catch::Approx(0.0f).margin(1e-6f));
}

TEST_CASE("Angle between perpendicular vectors is pi/2", "[vector_utils][angle]") {
    float const half_pi = 1.5707963267948966f;
    FloatVec2 right{ 1.0f, 0.0f };
    FloatVec2 up{ 0.0f, 1.0f };
    REQUIRE(Angle(right, up) == Catch::Approx(half_pi).epsilon(0.001));
}
