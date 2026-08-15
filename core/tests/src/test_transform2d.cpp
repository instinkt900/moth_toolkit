#include <catch2/catch_test_macros.hpp>

#include <moth/core/transform2d.h>

#include <cmath>

using namespace moth::core;

namespace {
    bool Near(float a, float b, float eps = 1e-4f) {
        return std::fabs(a - b) < eps;
    }
}

TEST_CASE("Transform2D: identity leaves points unchanged") {
    auto const t = Transform2D::Identity();
    auto const p = t.TransformPoint({ 3.0f, 4.0f });
    CHECK(Near(p.x, 3.0f));
    CHECK(Near(p.y, 4.0f));
}

TEST_CASE("Transform2D: translation") {
    Transform2D t;
    t.position = { 10.0f, 20.0f };
    auto const p = t.TransformPoint({ 1.0f, 2.0f });
    CHECK(Near(p.x, 11.0f));
    CHECK(Near(p.y, 22.0f));
}

TEST_CASE("Transform2D: scale about origin") {
    Transform2D t;
    t.scale = { 2.0f, 3.0f };
    auto const p = t.TransformPoint({ 1.0f, 1.0f });
    CHECK(Near(p.x, 2.0f));
    CHECK(Near(p.y, 3.0f));
}

TEST_CASE("Transform2D: rotation") {
    Transform2D t;
    t.rotation = 90.0f;
    auto const p = t.TransformPoint({ 1.0f, 0.0f });
    CHECK(Near(p.x, 0.0f));
    CHECK(Near(p.y, 1.0f));
}

TEST_CASE("Transform2D: pivot lands on position") {
    Transform2D t;
    t.position = { 100.0f, 50.0f };
    auto const matrix = t.ToMatrix({ 5.0f, 5.0f });
    auto const p = matrix.TransformPoint({ 5.0f, 5.0f });
    CHECK(Near(p.x, 100.0f));
    CHECK(Near(p.y, 50.0f));
}

TEST_CASE("Transform2D: pivot with scale keeps pivot anchored") {
    Transform2D t;
    t.position = { 100.0f, 50.0f };
    t.scale = { 2.0f, 2.0f };
    auto const matrix = t.ToMatrix({ 10.0f, 10.0f });
    auto const p = matrix.TransformPoint({ 10.0f, 10.0f });
    CHECK(Near(p.x, 100.0f));
    CHECK(Near(p.y, 50.0f));
}
