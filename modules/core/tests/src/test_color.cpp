#include <catch2/catch_all.hpp>

#include <moth/core/color.h>

using namespace moth::core;

namespace {
    void CheckColor(Color const& c, float r, float g, float b, float a) {
        CHECK(c.r == Catch::Approx(r));
        CHECK(c.g == Catch::Approx(g));
        CHECK(c.b == Catch::Approx(b));
        CHECK(c.a == Catch::Approx(a));
    }
}

TEST_CASE("Blend Replace overwrites destination", "[color][blend]") {
    Color const src{ 0.1f, 0.2f, 0.3f, 0.4f };
    Color const dst{ 0.9f, 0.8f, 0.7f, 0.6f };
    CheckColor(Blend(src, dst, BlendMode::Replace), 0.1f, 0.2f, 0.3f, 0.4f);
}

TEST_CASE("Blend Alpha composites source over destination", "[color][blend]") {
    Color const src{ 0.5f, 0.5f, 0.5f, 0.5f };
    Color const dst{ 1.0f, 1.0f, 1.0f, 1.0f };
    CheckColor(Blend(src, dst, BlendMode::Alpha), 0.75f, 0.75f, 0.75f, 1.0f);
}

TEST_CASE("Blend Add accumulates source onto destination", "[color][blend]") {
    Color const src{ 0.5f, 0.5f, 0.5f, 0.5f };
    Color const dst{ 0.2f, 0.2f, 0.2f, 1.0f };
    CheckColor(Blend(src, dst, BlendMode::Add), 0.45f, 0.45f, 0.45f, 1.0f);
}

TEST_CASE("Blend Multiply multiplies source and destination", "[color][blend]") {
    Color const src{ 0.5f, 0.5f, 0.5f, 0.5f };
    Color const dst{ 0.4f, 0.4f, 0.4f, 1.0f };
    CheckColor(Blend(src, dst, BlendMode::Multiply), 0.4f, 0.4f, 0.4f, 1.0f);
}

TEST_CASE("Blend Modulate modulates destination by source", "[color][blend]") {
    Color const src{ 0.5f, 0.5f, 0.5f, 0.5f };
    Color const dst{ 0.4f, 0.4f, 0.4f, 1.0f };
    CheckColor(Blend(src, dst, BlendMode::Modulate), 0.2f, 0.2f, 0.2f, 1.0f);
}
