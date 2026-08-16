#include "moth_graphics/graphics/camera.h"

#include <catch2/catch_all.hpp>

#include <cmath>

using namespace moth::gfx;

namespace {
    bool Near(float a, float b, float eps = 1e-3f) {
        return std::fabs(a - b) < eps;
    }

    FloatVec2 const kViewport{ 1280.0f, 720.0f };
}

TEST_CASE("Camera: default view maps origin to viewport centre", "[camera]") {
    Camera cam;
    auto const screen = cam.WorldToScreen({ 0.0f, 0.0f }, kViewport);
    CHECK(Near(screen.x, 640.0f));
    CHECK(Near(screen.y, 360.0f));
}

TEST_CASE("Camera: position offsets world origin", "[camera]") {
    Camera cam;
    cam.SetPosition({ 100.0f, 50.0f });
    auto const screen = cam.WorldToScreen({ 0.0f, 0.0f }, kViewport);
    CHECK(Near(screen.x, 640.0f - 100.0f));
    CHECK(Near(screen.y, 360.0f - 50.0f));
}

TEST_CASE("Camera: zoom scales the world", "[camera]") {
    Camera cam;
    cam.SetZoom(2.0f);
    // A world point 10 units right of origin is 20 screen pixels right of centre.
    auto const screen = cam.WorldToScreen({ 10.0f, 0.0f }, kViewport);
    CHECK(Near(screen.x, 640.0f + 20.0f));
    CHECK(Near(screen.y, 360.0f));
}

TEST_CASE("Camera: screen<->world round-trips through zoom and rotation", "[camera]") {
    Camera cam;
    cam.SetPosition({ 123.0f, -45.0f });
    cam.SetZoom(1.7f);
    cam.SetRotation(37.0f);

    FloatVec2 const world{ 42.0f, 88.0f };
    auto const screen = cam.WorldToScreen(world, kViewport);
    auto const back = cam.ScreenToWorld(screen, kViewport);
    CHECK(Near(back.x, world.x));
    CHECK(Near(back.y, world.y));
}

TEST_CASE("Camera: Follow snaps when smoothing is non-positive", "[camera]") {
    Camera cam;
    cam.SetPosition({ 0.0f, 0.0f });
    cam.Follow({ 50.0f, 60.0f }, 0.016f, 0.0f);
    CHECK(Near(cam.GetPosition().x, 50.0f));
    CHECK(Near(cam.GetPosition().y, 60.0f));
}

TEST_CASE("Camera: Follow converges toward the target", "[camera]") {
    Camera cam;
    cam.SetPosition({ 0.0f, 0.0f });
    for (int i = 0; i < 120; ++i) {
        cam.Follow({ 100.0f, 0.0f }, 1.0f / 60.0f, 10.0f);
    }
    // Exponential smoothing approaches but never quite reaches the target.
    CHECK(cam.GetPosition().x > 90.0f);
    CHECK(cam.GetPosition().x < 100.0f);
    CHECK(Near(cam.GetPosition().y, 0.0f));
}

TEST_CASE("Camera: shake decays to zero", "[camera]") {
    Camera cam;
    cam.SetPosition({ 0.0f, 0.0f });
    cam.Shake(10.0f, 1.0f);

    float maxOffset = 0.0f;
    for (int i = 0; i < 120; ++i) {
        cam.Update(1.0f / 60.0f);
        auto const offset = cam.GetShakeOffset();
        maxOffset = std::max(maxOffset, std::fabs(offset.x) + std::fabs(offset.y));
    }
    // Shake offset has fully decayed after its duration (1s at 60 Hz).
    CHECK(Near(cam.GetShakeOffset().x, 0.0f));
    CHECK(Near(cam.GetShakeOffset().y, 0.0f));
    // And it produced some displacement during the shake.
    CHECK(maxOffset > 0.0f);
}
