#pragma once

// NOLINTBEGIN

#include <array>
#include <cassert>

#undef M_PI
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

namespace moth_ui {
    // Smoothing functions from http://easings.net/ (With visual examples)

    /**
     * @brief Easing curve type used to interpolate between keyframe values.
     *
     * Each enumerator corresponds to a standard easing function; see
     * https://easings.net/ for visual examples.
     */
    enum class InterpType {
        Unknown,     ///< Unknown; treated as Linear at runtime.

        Step,        ///< Jumps immediately to the target value.
        Linear,      ///< Linear interpolation (no easing).
        Smooth,      ///< Smooth-step (cubic Hermite).

        // sin
        SineIn,      ///< Sine ease-in.
        SineOut,     ///< Sine ease-out.
        SineInOut,   ///< Sine ease-in-out.

        // quad
        QuadIn,      ///< Quadratic ease-in.
        QuadOut,     ///< Quadratic ease-out.
        QuadInOut,   ///< Quadratic ease-in-out.

        // cubic
        CubicIn,     ///< Cubic ease-in.
        CubicOut,    ///< Cubic ease-out.
        CubicInOut,  ///< Cubic ease-in-out.

        // quart
        QuartIn,     ///< Quartic ease-in.
        QuartOut,    ///< Quartic ease-out.
        QuartInOut,  ///< Quartic ease-in-out.

        // quint
        QuintIn,     ///< Quintic ease-in.
        QuintOut,    ///< Quintic ease-out.
        QuintInOut,  ///< Quintic ease-in-out.

        // expo
        ExpoIn,      ///< Exponential ease-in.
        ExpoOut,     ///< Exponential ease-out.
        ExpoInOut,   ///< Exponential ease-in-out.

        // circ
        CircIn,      ///< Circular ease-in.
        CircOut,     ///< Circular ease-out.
        CircInOut,   ///< Circular ease-in-out.

        // back
        BackIn,      ///< Back ease-in (slight overshoot at start).
        BackOut,     ///< Back ease-out (slight overshoot at end).
        BackInOut,   ///< Back ease-in-out.

        // elastic
        ElasticIn,   ///< Elastic ease-in.
        ElasticOut,  ///< Elastic ease-out.
        ElasticInOut,///< Elastic ease-in-out.

        // bounds
        BounceIn,    ///< Bounce ease-in.
        BounceOut,   ///< Bounce ease-out.
        BounceInOut, ///< Bounce ease-in-out.
    };

    static float constexpr F_PI = static_cast<float>(M_PI);

    /// @brief Step easing: always returns 0 (value jumps at t==1).
    inline float easeStep(float x) {
        return 0.0f;
    }

    /// @brief Linear easing: returns @p x unchanged.
    inline float easeLinear(float x) {
        return x;
    }

    /// @brief Smooth-step easing: cubic Hermite curve.
    inline float easeSmooth(float x) {
        return x * x * (3.0f - 2.0f * x);
    }

    /// @brief Sine ease-in.
    inline float easeSineIn(float x) {
        return 1.0f - std::cos((x * F_PI) / 2.0f);
    }

    /// @brief Sine ease-out.
    inline float easeSineOut(float x) {
        return std::sin((x * F_PI) / 2.0f);
    }

    /// @brief Sine ease-in-out.
    inline float easeSineInOut(float x) {
        return -(std::cos(F_PI * x) - 1.0f) / 2.0f;
    }

    /// @brief Quadratic ease-in.
    inline float easeQuadIn(float x) {
        return x * x;
    }

    /// @brief Quadratic ease-out.
    inline float easeQuadOut(float x) {
        return 1.0f - (1.0f - x) * (1.0f - x);
    }

    /// @brief Quadratic ease-in-out.
    inline float easeQuadInOut(float x) {
        return (x < 0.5f) ? (2.0f * x * x) : (1.0f - std::pow(-2.0f * x + 2.0f, 2.0f) / 2.0f);
    }

    /// @brief Cubic ease-in.
    inline float easeCubicIn(float x) {
        return x * x * x;
    }

    /// @brief Cubic ease-out.
    inline float easeCubicOut(float x) {
        return 1.0f - std::pow(1.0f - x, 3.0f);
    }

    /// @brief Cubic ease-in-out.
    inline float easeCubicInOut(float x) {
        return (x < 0.5f) ? (4.0f * x * x * x) : (1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f);
    }

    /// @brief Quartic ease-in.
    inline float easeQuartIn(float x) {
        return x * x * x * x;
    }

    /// @brief Quartic ease-out.
    inline float easeQuartOut(float x) {
        return 1.0f - std::pow(1.0f - x, 4.0f);
    }

    /// @brief Quartic ease-in-out.
    inline float easeQuartInOut(float x) {
        return (x < 0.5f) ? (8.0f * x * x * x * x) : (1.0f - std::pow(-2.0f * x + 2.0f, 4.0f) / 2.0f);
    }

    /// @brief Quintic ease-in.
    inline float easeQuintIn(float x) {
        return x * x * x * x * x;
    }

    /// @brief Quintic ease-out.
    inline float easeQuintOut(float x) {
        return 1.0f - std::pow(1.0f - x, 5.0f);
    }

    /// @brief Quintic ease-in-out.
    inline float easeQuintInOut(float x) {
        return (x < 0.5f) ? (16.0f * x * x * x * x * x) : (1.0f - std::pow(-2.0f * x + 2.0f, 5.0f) / 2.0f);
    }

    /// @brief Exponential ease-in.
    inline float easeExpoIn(float x) {
        return x == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * x - 10.0f);
    }

    /// @brief Exponential ease-out.
    inline float easeExpoOut(float x) {
        return x == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * x);
    }

    /// @brief Exponential ease-in-out.
    inline float easeExpoInOut(float x) {
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
               : x < 0.5f ? std::pow(2.0f, 20.0f * x - 10.0f) / 2.0f
                          : (2.0f - std::pow(2.0f, -20.0f * x + 10.0f)) / 2.0f;
    }

    /// @brief Circular ease-in.
    inline float easeCircIn(float x) {
        return 1.0f - std::sqrt(1.0f - std::pow(x, 2.0f));
    }

    /// @brief Circular ease-out.
    inline float easeCircOut(float x) {
        return std::sqrt(1.0f - std::pow(x - 1.0f, 2.0f));
    }

    /// @brief Circular ease-in-out.
    inline float easeCircInOut(float x) {
        return x < 0.5f
                   ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * x, 2.0f))) / 2.0f
                   : (std::sqrt(1.0f - std::pow(-2.0f * x + 2.0f, 2.0f)) + 1.0f) / 2.0f;
    }

    /// @brief Back ease-in (overshoots slightly at the start).
    inline float easeBackIn(float x) {
        float const c1 = 1.70158f;
        float const c3 = c1 + 1.0f;
        return c3 * x * x * x - c1 * x * x;
    }

    /// @brief Back ease-out (overshoots slightly at the end).
    inline float easeBackOut(float x) {
        float const c1 = 1.70158f;
        float const c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
    }

    /// @brief Back ease-in-out.
    inline float easeBackInOut(float x) {
        float const c1 = 1.70158f;
        float const c2 = c1 * 1.525f;
        return x < 0.5f
                   ? (std::pow(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) / 2.0f
                   : (std::pow(2.0f * x - 2.0f, 2.0f) * ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }

    /// @brief Elastic ease-in.
    inline float easeElasticIn(float x) {
        float const c4 = (2.0f * F_PI) / 3.0f;
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
                   : -std::pow(2.0f, 10.0f * x - 10.0f) * std::sin((x * 10.0f - 10.75f) * c4);
    }

    /// @brief Elastic ease-out.
    inline float easeElasticOut(float x) {
        float const c4 = (2.0f * F_PI) / 3.0f;
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
                   : std::pow(2.0f, -10.0f * x) * std::sin((x * 10.0f - 0.75f) * c4) + 1.0f;
    }

    /// @brief Elastic ease-in-out.
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

    /// @brief Bounce ease-in.
    inline float easeBounceIn(float x) {
        return 1 - easeBounceOut(1.0f - x);
    }

    /// @brief Bounce ease-out.
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

    /// @brief Bounce ease-in-out.
    inline float easeBounceInOut(float x) {
        return x < 0.5f
                   ? (1.0f - easeBounceOut(1.0f - 2.0f * x)) / 2.0f
                   : (1.0f + easeBounceOut(2.0f * x - 1.0f)) / 2.0f;
    }

    /// @brief Function pointer type for easing functions.
    typedef float (*EaseFunction)(float);

    /// @brief Array from InterpType to the corresponding easing function, indexed by enum value.
    inline constexpr std::array<EaseFunction, 34> InterpFuncs{{
        easeLinear,       // Unknown
        easeStep,         // Step
        easeLinear,       // Linear
        easeSmooth,       // Smooth
        easeSineIn,       // SineIn
        easeSineOut,      // SineOut
        easeSineInOut,    // SineInOut
        easeQuadIn,       // QuadIn
        easeQuadOut,      // QuadOut
        easeQuadInOut,    // QuadInOut
        easeCubicIn,      // CubicIn
        easeCubicOut,     // CubicOut
        easeCubicInOut,   // CubicInOut
        easeQuartIn,      // QuartIn
        easeQuartOut,     // QuartOut
        easeQuartInOut,   // QuartInOut
        easeQuintIn,      // QuintIn
        easeQuintOut,     // QuintOut
        easeQuintInOut,   // QuintInOut
        easeExpoIn,       // ExpoIn
        easeExpoOut,      // ExpoOut
        easeExpoInOut,    // ExpoInOut
        easeCircIn,       // CircIn
        easeCircOut,      // CircOut
        easeCircInOut,    // CircInOut
        easeBackIn,       // BackIn
        easeBackOut,      // BackOut
        easeBackInOut,    // BackInOut
        easeElasticIn,    // ElasticIn
        easeElasticOut,   // ElasticOut
        easeElasticInOut, // ElasticInOut
        easeBounceIn,     // BounceIn
        easeBounceOut,    // BounceOut
        easeBounceInOut,  // BounceInOut
    }};

    // If the enum grows, the array must stay in sync.
    static_assert(InterpFuncs.size() == static_cast<size_t>(InterpType::BounceInOut) + 1);

    /**
     * @brief Interpolates between two values using the specified easing curve.
     * @param a    Start value (returned when @p t == 0).
     * @param b    End value (returned when @p t == 1).
     * @param t    Normalised position in [0, 1].
     * @param type Easing curve to apply.
     * @return Interpolated value.
     */
    template <typename T>
    T Interp(T a, T b, float t, InterpType type) {
        assert(type != InterpType::Unknown && "Unknown interp type should never be used.");
        auto const interpFunc = InterpFuncs[static_cast<size_t>(type)];
        return (t == 0.0f) ? a : (t == 1.0f) ? b : (a + (b - a) * interpFunc(t));
    }
}
// NOLINTEND
