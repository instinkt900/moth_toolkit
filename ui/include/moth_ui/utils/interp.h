#pragma once

#undef M_PI
#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

namespace moth_ui {
    // Smoothing functions from http://easings.net/ (With visual examples)

    enum class InterpType {
        Unknown,

        Step,
        Linear,
        Smooth,

        // sin
        SineIn,
        SineOut,
        SineInOut,

        // quad
        QuadIn,
        QuadOut,
        QuadInOut,

        // cubic
        CubicIn,
        CubicOut,
        CubicInOut,

        // quart
        QuartIn,
        QuartOut,
        QuartInOut,

        // quint
        QuintIn,
        QuintOut,
        QuintInOut,

        // expo
        ExpoIn,
        ExpoOut,
        ExpoInOut,

        // circ
        CircIn,
        CircOut,
        CircInOut,

        // back
        BackIn,
        BackOut,
        BackInOut,

        // elastic
        ElasticIn,
        ElasticOut,
        ElasticInOut,

        // bounds
        BounceIn,
        BounceOut,
        BounceInOut,
    };

    static float constexpr F_PI = static_cast<float>(M_PI);

    inline float easeStep(float x) {
        return 0.0f;
    }

    inline float easeLinear(float x) {
        return x;
    }

    inline float easeSmooth(float x) {
        return x * x * (3.0f - 2.0f * x);
    }

    inline float easeSineIn(float x) {
        return 1.0f - std::cos((x * F_PI) / 2.0f);
    }

    inline float easeSineOut(float x) {
        return std::sin((x * F_PI) / 2.0f);
    }

    inline float easeSineInOut(float x) {
        return -(std::cos(F_PI * x) - 1.0f) / 2.0f;
    }

    inline float easeQuadIn(float x) {
        return x * x;
    }

    inline float easeQuadOut(float x) {
        return 1.0f - (1.0f - x) * (1.0f - x);
    }

    inline float easeQuadInOut(float x) {
        return (x < 0.5f) ? (2.0f * x * x) : (1.0f - std::pow(-2.0f * x + 2.0f, 2.0f) / 2.0f);
    }

    inline float easeCubicIn(float x) {
        return x * x * x;
    }

    inline float easeCubicOut(float x) {
        return 1.0f - std::pow(1.0f - x, 3.0f);
    }

    inline float easeCubicInOut(float x) {
        return (x < 0.5f) ? (4.0f * x * x * x) : (1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f);
    }

    inline float easeQuartIn(float x) {
        return x * x * x * x;
    }

    inline float easeQuartOut(float x) {
        return 1.0f - std::pow(1.0f - x, 4.0f);
    }

    inline float easeQuartInOut(float x) {
        return (x < 0.5f) ? (8.0f * x * x * x * x) : (1.0f - std::pow(-2.0f * x + 2.0f, 4.0f) / 2.0f);
    }

    inline float easeQuintIn(float x) {
        return x * x * x * x * x;
    }

    inline float easeQuintOut(float x) {
        return 1.0f - std::pow(1.0f - x, 5.0f);
    }

    inline float easeQuintInOut(float x) {
        return (x < 0.5f) ? (16.0f * x * x * x * x * x) : (1.0f - std::pow(-2.0f * x + 2.0f, 5.0f) / 2.0f);
    }

    inline float easeExpoIn(float x) {
        return x == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * x - 10.0f);
    }

    inline float easeExpoOut(float x) {
        return x == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * x);
    }

    inline float easeExpoInOut(float x) {
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
               : x < 0.5f ? std::pow(2.0f, 20.0f * x - 10.0f) / 2.0f
                          : (2.0f - std::pow(2.0f, -20.0f * x + 10.0f)) / 2.0f;
    }

    inline float easeCircIn(float x) {
        return 1.0f - std::sqrt(1.0f - std::pow(x, 2.0f));
    }

    inline float easeCircOut(float x) {
        return std::sqrt(1.0f - std::pow(x - 1.0f, 2.0f));
    }

    inline float easeCircInOut(float x) {
        return x < 0.5f
                   ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * x, 2.0f))) / 2.0f
                   : (std::sqrt(1.0f - std::pow(-2.0f * x + 2.0f, 2.0f)) + 1.0f) / 2.0f;
    }

    inline float easeBackIn(float x) {
        float const c1 = 1.70158f;
        float const c3 = c1 + 1.0f;
        return c3 * x * x * x - c1 * x * x;
    }

    inline float easeBackOut(float x) {
        float const c1 = 1.70158f;
        float const c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
    }

    inline float easeBackInOut(float x) {
        float const c1 = 1.70158f;
        float const c2 = c1 * 1.525f;
        return x < 0.5f
                   ? (std::pow(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) / 2.0f
                   : (std::pow(2.0f * x - 2.0f, 2.0f) * ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }

    inline float easeElasticIn(float x) {
        float const c4 = (2.0f * F_PI) / 3.0f;
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
                   : -std::pow(2.0f, 10.0f * x - 10.0f) * std::sin((x * 10.0f - 10.75f) * c4);
    }

    inline float easeElasticOut(float x) {
        float const c4 = (2.0f * F_PI) / 3.0f;
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
                   : std::pow(2.0f, -10.0f * x) * std::sin((x * 10.0f - 0.75f) * c4) + 1.0f;
    }

    inline float easeElasticInOut(float x) {
        float const c5 = (2.0f * F_PI) / 4.5f;
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
               : x < 0.5f
                   ? -(std::pow(2.0f, 20.0f * x - 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) / 2.0f
                   : (std::pow(2.0f, -20.0f * x + 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) / 2.0f + 1.0f;
    }

    float easeBounceOut(float x);

    inline float easeBounceIn(float x) {
        return 1 - easeBounceOut(1.0f - x);
    }

    inline float easeBounceOut(float x) {
        float const n1 = 7.5625f;
        float const d1 = 2.75f;

        if (x < 1.0f / d1) {
            return n1 * x * x;
        } else if (x < 2.0f / d1) {
	    x -= 1.5f / d1;
            return n1 * x * x + 0.75f;
        } else if (x < 2.5f / d1) {
	    x -= 2.25f / d1;
            return n1 * x * x + 0.9375f;
        } else {
	    x -= 2.625f / d1;
            return n1 * x * x + 0.984375f;
        }
    }

    inline float easeBounceInOut(float x) {
        return x < 0.5f
                   ? (1.0f - easeBounceOut(1.0f - 2.0f * x)) / 2.0f
                   : (1.0f + easeBounceOut(2.0f * x - 1.0f)) / 2.0f;
    }

    typedef float (*EaseFunction)(float);
    static std::map<InterpType, EaseFunction> const InterpFuncs{
        { InterpType::Step, easeStep },
        { InterpType::Linear, easeLinear },
        { InterpType::Smooth, easeSmooth },
        { InterpType::SineIn, easeSineIn },
        { InterpType::SineOut, easeSineOut },
        { InterpType::SineInOut, easeSineInOut },
        { InterpType::QuadIn, easeQuadIn },
        { InterpType::QuadOut, easeQuadOut },
        { InterpType::QuadInOut, easeQuadInOut },
        { InterpType::CubicIn, easeCubicIn },
        { InterpType::CubicOut, easeCubicOut },
        { InterpType::CubicInOut, easeCubicInOut },
        { InterpType::QuartIn, easeQuartIn },
        { InterpType::QuartOut, easeQuartOut },
        { InterpType::QuartInOut, easeQuartInOut },
        { InterpType::QuintIn, easeQuartIn },
        { InterpType::QuintOut, easeQuartOut },
        { InterpType::QuintInOut, easeQuartInOut },
        { InterpType::ExpoIn, easeExpoIn },
        { InterpType::ExpoOut, easeExpoOut },
        { InterpType::ExpoInOut, easeExpoInOut },
        { InterpType::CircIn, easeCircIn },
        { InterpType::CircOut, easeCircOut },
        { InterpType::CircInOut, easeCircInOut },
        { InterpType::BackIn, easeBackIn },
        { InterpType::BackOut, easeBackOut },
        { InterpType::BackInOut, easeBackInOut },
        { InterpType::ElasticIn, easeElasticIn },
        { InterpType::ElasticOut, easeElasticOut },
        { InterpType::ElasticInOut, easeElasticInOut },
        { InterpType::BounceIn, easeBounceIn },
        { InterpType::BounceOut, easeBounceOut },
        { InterpType::BounceInOut, easeBounceInOut },
    };

    template <typename T>
    T Interp(T a, T b, float t, InterpType type) {
        auto const interpFunc = InterpFuncs.at(type);
        return (t == 0.0f) ? a : (t == 1.0f) ? b : (a + (b - a) * interpFunc(t));
    }
}
