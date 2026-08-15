#include "moth/core/noise.h"

#include <catch2/catch_all.hpp>

#include <algorithm>
#include <cmath>

using namespace moth::core;

TEST_CASE("PerlinNoise: zero at integer lattice points", "[core][noise]") {
    PerlinNoise noise(123u);
    REQUIRE(noise.Noise(0.0f, 0.0f) == Catch::Approx(0.0f).margin(0.0001f));
    REQUIRE(noise.Noise(5.0f, -3.0f) == Catch::Approx(0.0f).margin(0.0001f));
    REQUIRE(noise.Noise(-2.0f, 8.0f) == Catch::Approx(0.0f).margin(0.0001f));
}

TEST_CASE("PerlinNoise: deterministic for a given seed", "[core][noise]") {
    PerlinNoise a(7u);
    PerlinNoise b(7u);
    for (int i = 0; i < 100; ++i) {
        float const x = static_cast<float>(i) * 0.13f;
        float const y = static_cast<float>(i) * 0.07f;
        REQUIRE(a.Noise(x, y) == b.Noise(x, y));
    }
}

TEST_CASE("PerlinNoise: different seeds give different noise", "[core][noise]") {
    PerlinNoise a(1u);
    PerlinNoise b(2u);
    bool differs = false;
    for (int i = 0; i < 100 && !differs; ++i) {
        float const x = static_cast<float>(i) * 0.5f;
        float const y = static_cast<float>(i) * 0.3f;
        if (a.Noise(x, y) != b.Noise(x, y)) {
            differs = true;
        }
    }
    REQUIRE(differs);
}

TEST_CASE("PerlinNoise: output stays within [-1, 1]", "[core][noise]") {
    PerlinNoise noise(5u);
    for (int i = 0; i < 10000; ++i) {
        float const x = static_cast<float>(i) * 0.017f;
        float const y = static_cast<float>(i) * 0.011f;
        float const v = noise.Noise(x, y);
        REQUIRE(v >= -1.0f);
        REQUIRE(v <= 1.0f);
    }
}

TEST_CASE("PerlinNoise: smooth — small input change gives small output change", "[core][noise]") {
    PerlinNoise noise(9u);
    constexpr float dx = 0.01f;
    for (int i = 0; i < 100; ++i) {
        float const x = static_cast<float>(i) * 0.37f;
        float const y = static_cast<float>(i) * 0.23f;
        float const delta = std::fabs(noise.Noise(x + dx, y) - noise.Noise(x, y));
        REQUIRE(delta < 0.2f);
    }
}

TEST_CASE("PerlinNoise: Fractal is normalized to [-1, 1]", "[core][noise]") {
    PerlinNoise noise(11u);
    for (int i = 0; i < 1000; ++i) {
        float const x = static_cast<float>(i) * 0.031f;
        float const y = static_cast<float>(i) * 0.019f;
        float const v = noise.Fractal(x, y, 4);
        REQUIRE(v >= -1.0f);
        REQUIRE(v <= 1.0f);
    }
}

TEST_CASE("SimplexNoise: deterministic for a given seed", "[core][noise]") {
    SimplexNoise a(13u);
    SimplexNoise b(13u);
    for (int i = 0; i < 100; ++i) {
        float const x = static_cast<float>(i) * 0.11f;
        float const y = static_cast<float>(i) * 0.05f;
        REQUIRE(a.Noise(x, y) == b.Noise(x, y));
    }
}

TEST_CASE("SimplexNoise: output stays within [-1, 1]", "[core][noise]") {
    SimplexNoise noise(17u);
    for (int i = 0; i < 10000; ++i) {
        float const x = static_cast<float>(i) * 0.013f;
        float const y = static_cast<float>(i) * 0.009f;
        float const v = noise.Noise(x, y);
        REQUIRE(v >= -1.0f);
        REQUIRE(v <= 1.0f);
    }
}

TEST_CASE("SimplexNoise: smooth — small input change gives small output change", "[core][noise]") {
    SimplexNoise noise(19u);
    constexpr float dx = 0.01f;
    for (int i = 0; i < 100; ++i) {
        float const x = static_cast<float>(i) * 0.29f;
        float const y = static_cast<float>(i) * 0.17f;
        float const delta = std::fabs(noise.Noise(x + dx, y) - noise.Noise(x, y));
        REQUIRE(delta < 0.2f);
    }
}

TEST_CASE("Noise: produces variation, not a constant", "[core][noise]") {
    PerlinNoise perlin(3u);
    SimplexNoise simplex(3u);
    float minP = 1e9f, maxP = -1e9f;
    float minS = 1e9f, maxS = -1e9f;
    for (int i = 0; i < 1000; ++i) {
        float const x = static_cast<float>(i) * 0.1f;
        float const y = static_cast<float>(i) * 0.07f;
        float const p = perlin.Noise(x, y);
        float const s = simplex.Noise(x, y);
        minP = std::min(minP, p);
        maxP = std::max(maxP, p);
        minS = std::min(minS, s);
        maxS = std::max(maxS, s);
    }
    REQUIRE(maxP - minP > 0.01f);
    REQUIRE(maxS - minS > 0.01f);
}
