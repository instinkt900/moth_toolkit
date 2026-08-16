#include <catch2/catch_test_macros.hpp>

#include <moth/core/geometry.h>

#include <cmath>

using namespace moth::core;

namespace {
    bool Near(float a, float b, float eps = 1e-3f) {
        return std::fabs(a - b) < eps;
    }
}

TEST_CASE("PointInCircle", "[geometry][circle]") {
    Circle const c{ { 0.0f, 0.0f }, 5.0f };
    CHECK(PointInCircle({ 0.0f, 0.0f }, c));
    CHECK(PointInCircle({ 5.0f, 0.0f }, c));
    CHECK_FALSE(PointInCircle({ 5.1f, 0.0f }, c));
}

TEST_CASE("Circle-circle intersection", "[geometry][circle]") {
    Circle const a{ { 0.0f, 0.0f }, 5.0f };
    Circle const b{ { 8.0f, 0.0f }, 3.0f };   // touching (5+3=8)
    Circle const c{ { 9.0f, 0.0f }, 3.0f };   // gap
    Circle const d{ { 2.0f, 0.0f }, 3.0f };   // overlap
    CHECK(Intersects(a, b));
    CHECK_FALSE(Intersects(a, c));
    CHECK(Intersects(a, d));
}

TEST_CASE("Circle-AABB intersection", "[geometry][circle]") {
    AABB const box{ { 0.0f, 0.0f }, { 5.0f, 5.0f } };
    CHECK(Intersects(Circle{ { 0.0f, 0.0f }, 1.0f }, box));    // inside
    CHECK(Intersects(Circle{ { 8.0f, 0.0f }, 4.0f }, box));    // overlaps edge
    CHECK_FALSE(Intersects(Circle{ { 20.0f, 0.0f }, 1.0f }, box));
}

TEST_CASE("Circle-rect intersection", "[geometry][circle]") {
    FloatRect const rect{ { 0.0f, 0.0f }, { 10.0f, 10.0f } };
    CHECK(Intersects(Circle{ { 5.0f, 5.0f }, 2.0f }, rect));
    CHECK_FALSE(Intersects(Circle{ { 30.0f, 30.0f }, 1.0f }, rect));
}

TEST_CASE("Segment-circle intersection", "[geometry][segment]") {
    Circle const circle{ { 0.0f, 0.0f }, 5.0f };
    CHECK(Intersects(Segment{ { -10.0f, 0.0f }, { 10.0f, 0.0f } }, circle)); // through centre
    CHECK(Intersects(Segment{ { -10.0f, 0.0f }, { -4.0f, 0.0f } }, circle)); // touches edge
    CHECK_FALSE(Intersects(Segment{ { -10.0f, 10.0f }, { 10.0f, 10.0f } }, circle)); // misses
    CHECK_FALSE(Intersects(Segment{ { 10.0f, 0.0f }, { 20.0f, 0.0f } }, circle));    // points away
}

TEST_CASE("Segment-segment intersection", "[geometry][segment]") {
    // Cross.
    CHECK(Intersects(Segment{ { -1.0f, 0.0f }, { 1.0f, 0.0f } }, Segment{ { 0.0f, -1.0f }, { 0.0f, 1.0f } }));
    // Collinear overlapping.
    CHECK(Intersects(Segment{ { 0.0f, 0.0f }, { 10.0f, 0.0f } }, Segment{ { 5.0f, 0.0f }, { 15.0f, 0.0f } }));
    // Endpoint touch.
    CHECK(Intersects(Segment{ { 0.0f, 0.0f }, { 5.0f, 5.0f } }, Segment{ { 5.0f, 5.0f }, { 9.0f, 9.0f } }));
    // Disjoint.
    CHECK_FALSE(Intersects(Segment{ { 0.0f, 0.0f }, { 1.0f, 0.0f } }, Segment{ { 0.0f, 1.0f }, { 1.0f, 1.0f } }));
}

TEST_CASE("RaycastCircle hits and normals", "[geometry][raycast]") {
    Circle const circle{ { 0.0f, 0.0f }, 5.0f };
    Ray const ray{ { -20.0f, 0.0f }, { 1.0f, 0.0f } };

    float t = 0.0f;
    FloatVec2 normal;
    REQUIRE(RaycastCircle(ray, circle, &t, &normal));
    CHECK(Near(t, 15.0f));           // centre is 20 away, radius 5 -> hit at 15
    CHECK(Near(normal.x, -1.0f));    // hit the -x face
    CHECK(Near(normal.y, 0.0f));
}

TEST_CASE("RaycastCircle misses when pointing away", "[geometry][raycast]") {
    Circle const circle{ { 0.0f, 0.0f }, 5.0f };
    Ray const ray{ { 20.0f, 0.0f }, { 1.0f, 0.0f } }; // behind the circle, pointing away
    CHECK_FALSE(RaycastCircle(ray, circle));
}

TEST_CASE("RaycastAABB hits and normals", "[geometry][raycast]") {
    AABB const box{ { 0.0f, 0.0f }, { 5.0f, 5.0f } };
    Ray const ray{ { -20.0f, 2.0f }, { 1.0f, 0.0f } };

    float t = 0.0f;
    FloatVec2 normal;
    REQUIRE(RaycastAABB(ray, box, &t, &normal));
    CHECK(Near(t, 15.0f));           // left face at x=-5, origin -20 -> 15
    CHECK(Near(normal.x, -1.0f));
    CHECK(Near(normal.y, 0.0f));
}

TEST_CASE("RaycastAABB misses", "[geometry][raycast]") {
    AABB const box{ { 0.0f, 0.0f }, { 5.0f, 5.0f } };
    CHECK_FALSE(RaycastAABB(Ray{ { 0.0f, 20.0f }, { 1.0f, 0.0f } }, box)); // parallel, above
    CHECK_FALSE(RaycastAABB(Ray{ { 20.0f, 2.0f }, { 1.0f, 0.0f } }, box));  // pointing away
}

TEST_CASE("RaycastRect delegates to AABB", "[geometry][raycast]") {
    FloatRect const rect{ { -5.0f, -5.0f }, { 5.0f, 5.0f } };
    Ray const ray{ { -20.0f, 0.0f }, { 1.0f, 0.0f } };

    float t = 0.0f;
    REQUIRE(RaycastRect(ray, rect, &t));
    CHECK(Near(t, 15.0f));
}
