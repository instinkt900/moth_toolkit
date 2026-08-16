#include "moth/core/random.h"

#include <catch2/catch_all.hpp>

using namespace moth::core;

TEST_CASE("Random: same seed reproduces the same sequence", "[core][random]") {
    Random a(1234u);
    Random b(1234u);
    for (int i = 0; i < 100; ++i) {
        REQUIRE(a.NextUInt() == b.NextUInt());
        REQUIRE(a.NextFloat() == b.NextFloat());
    }
}

TEST_CASE("Random: different seeds diverge", "[core][random]") {
    Random a(1u);
    Random b(2u);
    bool differs = false;
    for (int i = 0; i < 100; ++i) {
        if (a.NextUInt() != b.NextUInt()) {
            differs = true;
            break;
        }
    }
    REQUIRE(differs);
}

TEST_CASE("Random: Reseed restarts the sequence", "[core][random]") {
    Random r(99u);
    auto const first = r.NextUInt();
    r.Seed(99u);
    REQUIRE(r.NextUInt() == first);
}

TEST_CASE("Random: NextFloat is in [0, 1)", "[core][random]") {
    Random r(42u);
    for (int i = 0; i < 1000; ++i) {
        float const v = r.NextFloat();
        REQUIRE(v >= 0.0f);
        REQUIRE(v < 1.0f);
    }
}

TEST_CASE("Random: NextFloat(min, max) is in [min, max)", "[core][random]") {
    Random r(43u);
    for (int i = 0; i < 1000; ++i) {
        float const v = r.NextFloat(2.0f, 5.0f);
        REQUIRE(v >= 2.0f);
        REQUIRE(v < 5.0f);
    }
}

TEST_CASE("Random: NextInt is in [min, max]", "[core][random]") {
    Random r(44u);
    for (int i = 0; i < 1000; ++i) {
        int const v = r.NextInt(-3, 3);
        REQUIRE(v >= -3);
        REQUIRE(v <= 3);
    }
}

TEST_CASE("Random: NextUInt(bound) is in [0, bound)", "[core][random]") {
    Random r(45u);
    for (int i = 0; i < 1000; ++i) {
        std::uint64_t const v = r.NextUInt(10u);
        REQUIRE(v < 10u);
    }
}

TEST_CASE("Random: NextBool produces both outcomes", "[core][random]") {
    Random r(46u);
    bool sawTrue = false;
    bool sawFalse = false;
    for (int i = 0; i < 100; ++i) {
        if (r.NextBool()) {
            sawTrue = true;
        } else {
            sawFalse = true;
        }
    }
    REQUIRE(sawTrue);
    REQUIRE(sawFalse);
}
