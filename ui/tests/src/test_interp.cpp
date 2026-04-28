#include "moth_ui/utils/interp.h"
#include <catch2/catch_all.hpp>
#include <magic_enum.hpp>

using namespace moth_ui;

TEST_CASE("interpStep always returns 0", "[interp][step]") {
    REQUIRE(interpStep(0.0f) == 0.0f);
    REQUIRE(interpStep(0.5f) == 0.0f);
    REQUIRE(interpStep(1.0f) == 0.0f);
}

TEST_CASE("interpLinear is identity", "[interp][linear]") {
    REQUIRE(interpLinear(0.0f) == 0.0f);
    REQUIRE(interpLinear(0.5f) == 0.5f);
    REQUIRE(interpLinear(1.0f) == 1.0f);
    REQUIRE(interpLinear(0.25f) == 0.25f);
}

TEST_CASE("interpSmooth boundary and midpoint", "[interp][smooth]") {
    REQUIRE(interpSmooth(0.0f) == Catch::Approx(0.0f));
    REQUIRE(interpSmooth(0.5f) == Catch::Approx(0.5f));
    REQUIRE(interpSmooth(1.0f) == Catch::Approx(1.0f));
}

TEST_CASE("interpSmooth is slower at ends, faster in middle", "[interp][smooth]") {
    // Smooth curve starts and ends slower — derivative at 0/1 is 0
    // Mid-section is faster than linear
    REQUIRE(interpSmooth(0.5f) >= interpLinear(0.5f));
    // But at 0 and 1 they agree
    REQUIRE(interpSmooth(0.0f) == Catch::Approx(interpLinear(0.0f)));
    REQUIRE(interpSmooth(1.0f) == Catch::Approx(interpLinear(1.0f)));
}

TEST_CASE("All easing functions return 0 at t=0 and 1 at t=1", "[interp][boundaries]") {
    using EaseFn = float (*)(float);
    auto check = [](EaseFn fn, char const* name) {
        INFO("easing function: " << name);
        REQUIRE(fn(0.0f) == Catch::Approx(0.0f).margin(1e-5f));
        REQUIRE(fn(1.0f) == Catch::Approx(1.0f).margin(1e-5f));
    };

    check(interpLinear,      "interpLinear");
    check(interpSmooth,      "interpSmooth");
    check(interpSineIn,      "interpSineIn");
    check(interpSineOut,     "interpSineOut");
    check(interpSineInOut,   "interpSineInOut");
    check(interpQuadIn,      "interpQuadIn");
    check(interpQuadOut,     "interpQuadOut");
    check(interpQuadInOut,   "interpQuadInOut");
    check(interpCubicIn,     "interpCubicIn");
    check(interpCubicOut,    "interpCubicOut");
    check(interpCubicInOut,  "interpCubicInOut");
    check(interpQuartIn,     "interpQuartIn");
    check(interpQuartOut,    "interpQuartOut");
    check(interpQuartInOut,  "interpQuartInOut");
    check(interpQuintIn,     "interpQuintIn");
    check(interpQuintOut,    "interpQuintOut");
    check(interpQuintInOut,  "interpQuintInOut");
    check(interpExpoIn,      "interpExpoIn");
    check(interpExpoOut,     "interpExpoOut");
    check(interpExpoInOut,   "interpExpoInOut");
    check(interpCircIn,      "interpCircIn");
    check(interpCircOut,     "interpCircOut");
    check(interpCircInOut,   "interpCircInOut");
    check(interpElasticIn,   "interpElasticIn");
    check(interpElasticOut,  "interpElasticOut");
    check(interpElasticInOut,"interpElasticInOut");
    check(interpBounceIn,    "interpBounceIn");
    check(interpBounceOut,   "interpBounceOut");
    check(interpBounceInOut, "interpBounceInOut");
    check(interpBackIn,      "interpBackIn");
    check(interpBackOut,     "interpBackOut");
    check(interpBackInOut,   "interpBackInOut");
}

TEST_CASE("In and Out ease functions are complements", "[interp][symmetry]") {
    // easeXIn(t) == 1 - easeXOut(1 - t) for all t
    using EasePair = std::pair<float (*)(float), float (*)(float)>;
    std::initializer_list<EasePair> pairs = {
        { interpSineIn,    interpSineOut    },
        { interpQuadIn,    interpQuadOut    },
        { interpCubicIn,   interpCubicOut   },
        { interpQuartIn,   interpQuartOut   },
        { interpQuintIn,   interpQuintOut   },
        { interpExpoIn,    interpExpoOut    },
        { interpCircIn,    interpCircOut    },
        { interpBackIn,    interpBackOut    },
        { interpElasticIn, interpElasticOut },
        { interpBounceIn,  interpBounceOut  },
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

    // Every function must be non-null
    for (auto fn : InterpFuncs) {
        REQUIRE(fn != nullptr);
    }

    // Unknown defaults to linear
    REQUIRE(InterpFuncs[static_cast<size_t>(InterpType::Unknown)](0.5f) == Catch::Approx(0.5f));
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
    for (size_t i = 0; i < InterpFuncs.size(); ++i) {
        auto const type = static_cast<InterpType>(i);
        if (type == InterpType::Unknown) continue; // Unknown is a programming error; assert fires in debug
        REQUIRE(Interp(1.0f, 9.0f, 0.0f, type) == Catch::Approx(1.0f));
        REQUIRE(Interp(1.0f, 9.0f, 1.0f, type) == Catch::Approx(9.0f));
    }
}
