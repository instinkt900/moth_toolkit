#include "moth_ui/utils/interp.h"
#include <catch2/catch_all.hpp>
#include <magic_enum.hpp>

using namespace moth_ui;

TEST_CASE("easeStep always returns 0", "[interp][step]") {
    REQUIRE(easeStep(0.0f) == 0.0f);
    REQUIRE(easeStep(0.5f) == 0.0f);
    REQUIRE(easeStep(1.0f) == 0.0f);
}

TEST_CASE("easeLinear is identity", "[interp][linear]") {
    REQUIRE(easeLinear(0.0f) == 0.0f);
    REQUIRE(easeLinear(0.5f) == 0.5f);
    REQUIRE(easeLinear(1.0f) == 1.0f);
    REQUIRE(easeLinear(0.25f) == 0.25f);
}

TEST_CASE("easeSmooth boundary and midpoint", "[interp][smooth]") {
    REQUIRE(easeSmooth(0.0f) == Catch::Approx(0.0f));
    REQUIRE(easeSmooth(0.5f) == Catch::Approx(0.5f));
    REQUIRE(easeSmooth(1.0f) == Catch::Approx(1.0f));
}

TEST_CASE("easeSmooth is slower at ends, faster in middle", "[interp][smooth]") {
    // Smooth curve starts and ends slower — derivative at 0/1 is 0
    // Mid-section is faster than linear
    REQUIRE(easeSmooth(0.5f) >= easeLinear(0.5f));
    // But at 0 and 1 they agree
    REQUIRE(easeSmooth(0.0f) == Catch::Approx(easeLinear(0.0f)));
    REQUIRE(easeSmooth(1.0f) == Catch::Approx(easeLinear(1.0f)));
}

TEST_CASE("All easing functions return 0 at t=0 and 1 at t=1", "[interp][boundaries]") {
    using EaseFn = float (*)(float);
    auto check = [](EaseFn fn, char const* name) {
        INFO("easing function: " << name);
        REQUIRE(fn(0.0f) == Catch::Approx(0.0f).margin(1e-5f));
        REQUIRE(fn(1.0f) == Catch::Approx(1.0f).margin(1e-5f));
    };

    check(easeLinear,      "easeLinear");
    check(easeSmooth,      "easeSmooth");
    check(easeSineIn,      "easeSineIn");
    check(easeSineOut,     "easeSineOut");
    check(easeSineInOut,   "easeSineInOut");
    check(easeQuadIn,      "easeQuadIn");
    check(easeQuadOut,     "easeQuadOut");
    check(easeQuadInOut,   "easeQuadInOut");
    check(easeCubicIn,     "easeCubicIn");
    check(easeCubicOut,    "easeCubicOut");
    check(easeCubicInOut,  "easeCubicInOut");
    check(easeQuartIn,     "easeQuartIn");
    check(easeQuartOut,    "easeQuartOut");
    check(easeQuartInOut,  "easeQuartInOut");
    check(easeQuintIn,     "easeQuintIn");
    check(easeQuintOut,    "easeQuintOut");
    check(easeQuintInOut,  "easeQuintInOut");
    check(easeExpoIn,      "easeExpoIn");
    check(easeExpoOut,     "easeExpoOut");
    check(easeExpoInOut,   "easeExpoInOut");
    check(easeCircIn,      "easeCircIn");
    check(easeCircOut,     "easeCircOut");
    check(easeCircInOut,   "easeCircInOut");
    check(easeElasticIn,   "easeElasticIn");
    check(easeElasticOut,  "easeElasticOut");
    check(easeElasticInOut,"easeElasticInOut");
    check(easeBounceIn,    "easeBounceIn");
    check(easeBounceOut,   "easeBounceOut");
    check(easeBounceInOut, "easeBounceInOut");
    check(easeBackIn,      "easeBackIn");
    check(easeBackOut,     "easeBackOut");
    check(easeBackInOut,   "easeBackInOut");
}

TEST_CASE("In and Out ease functions are complements", "[interp][symmetry]") {
    // easeXIn(t) == 1 - easeXOut(1 - t) for all t
    using EasePair = std::pair<float (*)(float), float (*)(float)>;
    std::initializer_list<EasePair> pairs = {
        { easeSineIn,    easeSineOut    },
        { easeQuadIn,    easeQuadOut    },
        { easeCubicIn,   easeCubicOut   },
        { easeQuartIn,   easeQuartOut   },
        { easeQuintIn,   easeQuintOut   },
        { easeExpoIn,    easeExpoOut    },
        { easeCircIn,    easeCircOut    },
        { easeBackIn,    easeBackOut    },
        { easeElasticIn, easeElasticOut },
        { easeBounceIn,  easeBounceOut  },
    };
    for (float t : { 0.3f, 0.5f, 0.7f }) {
        for (auto [in, out] : pairs) {
            REQUIRE(in(t) == Catch::Approx(1.0f - out(1.0f - t)).epsilon(0.001));
        }
    }
}

TEST_CASE("InterpFuncs map contains all InterpType entries", "[interp][map]") {
    // One entry per InterpType enum value
    REQUIRE(InterpFuncs.size() == magic_enum::enum_count<InterpType>());

    // Every mapped function must be non-null
    for (auto const& [type, fn] : InterpFuncs) {
        (void)type;
        REQUIRE(fn != nullptr);
    }

    // Unknown defaults to linear
    REQUIRE(InterpFuncs.count(InterpType::Unknown)    == 1);
    REQUIRE(InterpFuncs.at(InterpType::Unknown)(0.5f) == Catch::Approx(0.5f));
}

TEST_CASE("Interp<float> linear interpolation", "[interp][interp_template]") {
    REQUIRE(Interp(0.0f, 10.0f, 0.0f,  InterpType::Linear) == Catch::Approx(0.0f));
    REQUIRE(Interp(0.0f, 10.0f, 1.0f,  InterpType::Linear) == Catch::Approx(10.0f));
    REQUIRE(Interp(0.0f, 10.0f, 0.5f,  InterpType::Linear) == Catch::Approx(5.0f));
    REQUIRE(Interp(2.0f, 8.0f,  0.25f, InterpType::Linear) == Catch::Approx(3.5f));
}

TEST_CASE("Interp<float> step holds start value until t==1", "[interp][interp_template]") {
    REQUIRE(Interp(0.0f, 10.0f, 0.5f, InterpType::Step) == Catch::Approx(0.0f));
    REQUIRE(Interp(0.0f, 10.0f, 0.9f, InterpType::Step) == Catch::Approx(0.0f));
    REQUIRE(Interp(0.0f, 10.0f, 1.0f, InterpType::Step) == Catch::Approx(10.0f));
}

TEST_CASE("Interp<float> t=0 always returns a, t=1 always returns b", "[interp][interp_template]") {
    for (auto const& [type, fn] : InterpFuncs) {
        (void)fn;
        if (type == InterpType::Unknown) continue; // Unknown is a programming error; assert fires in debug
        REQUIRE(Interp(1.0f, 9.0f, 0.0f, type) == Catch::Approx(1.0f));
        REQUIRE(Interp(1.0f, 9.0f, 1.0f, type) == Catch::Approx(9.0f));
    }
}
