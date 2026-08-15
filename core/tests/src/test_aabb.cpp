#include <catch2/catch_test_macros.hpp>

#include <moth/core/aabb.h>

#include <cmath>

using namespace moth::core;

namespace {
    bool Near(float a, float b, float eps = 1e-4f) {
        return std::fabs(a - b) < eps;
    }
}

TEST_CASE("AABB: construction from centre and half-extents", "[aabb]") {
    AABB const box{ { 10.0f, 20.0f }, { 5.0f, 7.0f } };
    CHECK(Near(box.center.x, 10.0f));
    CHECK(Near(box.center.y, 20.0f));
    CHECK(Near(box.GetMin().x, 5.0f));
    CHECK(Near(box.GetMin().y, 13.0f));
    CHECK(Near(box.GetMax().x, 15.0f));
    CHECK(Near(box.GetMax().y, 27.0f));
    CHECK(Near(box.GetSize().x, 10.0f));
    CHECK(Near(box.GetSize().y, 14.0f));
}

TEST_CASE("AABB: FromMinMax round-trips", "[aabb]") {
    AABB const box = AABB::FromMinMax({ 1.0f, 2.0f }, { 5.0f, 8.0f });
    CHECK(Near(box.center.x, 3.0f));
    CHECK(Near(box.center.y, 5.0f));
    CHECK(Near(box.halfExtents.x, 2.0f));
    CHECK(Near(box.halfExtents.y, 3.0f));
}

TEST_CASE("AABB: Contains point", "[aabb]") {
    AABB const box{ { 0.0f, 0.0f }, { 10.0f, 10.0f } };
    CHECK(box.Contains({ 0.0f, 0.0f }));
    CHECK(box.Contains({ 10.0f, 10.0f }));
    CHECK(box.Contains({ -10.0f, -10.0f }));
    CHECK_FALSE(box.Contains({ 11.0f, 0.0f }));
    CHECK_FALSE(box.Contains({ 0.0f, -11.0f }));
}

TEST_CASE("AABB: Contains box", "[aabb]") {
    AABB const outer{ { 0.0f, 0.0f }, { 10.0f, 10.0f } };
    AABB const inner{ { 1.0f, 1.0f }, { 2.0f, 2.0f } };
    CHECK(outer.Contains(inner));
    CHECK_FALSE(inner.Contains(outer));
}

TEST_CASE("AABB: Overlaps", "[aabb]") {
    AABB const a{ { 0.0f, 0.0f }, { 5.0f, 5.0f } };
    AABB const b{ { 5.0f, 5.0f }, { 5.0f, 5.0f } };   // touching corner
    AABB const c{ { 6.0f, 6.0f }, { 5.0f, 5.0f } };   // overlap
    AABB const d{ { 20.0f, 20.0f }, { 1.0f, 1.0f } }; // separate
    CHECK(a.Overlaps(b));
    CHECK(a.Overlaps(c));
    CHECK_FALSE(a.Overlaps(d));
}

TEST_CASE("AABB: ClosestPoint and distance", "[aabb]") {
    AABB const box{ { 0.0f, 0.0f }, { 5.0f, 5.0f } };
    auto const cp = box.ClosestPoint({ 20.0f, 0.0f });
    CHECK(Near(cp.x, 5.0f));
    CHECK(Near(cp.y, 0.0f));

    CHECK(Near(box.Distance({ 0.0f, 0.0f }), 0.0f));   // inside
    CHECK(Near(box.Distance({ 8.0f, 0.0f }), 3.0f));   // outside +x
    CHECK(Near(box.Distance({ 0.0f, -10.0f }), 5.0f)); // outside -y
}

TEST_CASE("AABB: Merged and Expanded", "[aabb]") {
    AABB const a{ { 0.0f, 0.0f }, { 5.0f, 5.0f } };
    AABB const b{ { 10.0f, 0.0f }, { 5.0f, 5.0f } };
    AABB const merged = a.Merged(b);
    CHECK(Near(merged.GetMin().x, -5.0f));
    CHECK(Near(merged.GetMax().x, 15.0f));

    AABB const grown = a.Expanded({ 2.0f, 3.0f });
    CHECK(Near(grown.halfExtents.x, 7.0f));
    CHECK(Near(grown.halfExtents.y, 8.0f));
}
