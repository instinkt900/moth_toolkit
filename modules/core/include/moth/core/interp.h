#pragma once

// NOLINTBEGIN

#include <array>
#include <cassert>
#include <cmath>

namespace moth::core {
    // Smoothing functions from http://easings.net/ (With visual examples)
    //
    // The polynomial curves are constexpr and written with multiplication
    // rather than std::pow: pow with a float exponent is a libm call the
    // compiler usually cannot fold (it may set errno), so spelling out the
    // multiplications is both faster at runtime and usable at compile time.
    // The sine, expo, circ and elastic families genuinely need <cmath> and so
    // stay runtime-only until C++26 makes those functions constexpr.

    /**
     * @brief Easing curve type used to interpolate between keyframe values.
     *
     * Each enumerator corresponds to a standard easing function; see
     * https://easings.net/ for visual examples.
     */
    enum class InterpType {
        Unknown,     ///< Unknown; invalid value — @c Interp() asserts if used.

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

    inline constexpr float F_PI = 3.14159265358979f;

    /// @brief Step easing: always returns 0 (value jumps at t==1).
    constexpr float interpStep(float x) {
        return 0.0f;
    }

    /// @brief Linear easing: returns @p x unchanged.
    constexpr float interpLinear(float x) {
        return x;
    }

    /// @brief Smooth-step easing: cubic Hermite curve.
    constexpr float interpSmooth(float x) {
        return x * x * (3.0f - 2.0f * x);
    }

    /// @brief Sine ease-in.
    inline float interpSineIn(float x) {
        return 1.0f - std::cos((x * F_PI) / 2.0f);
    }

    /// @brief Sine ease-out.
    inline float interpSineOut(float x) {
        return std::sin((x * F_PI) / 2.0f);
    }

    /// @brief Sine ease-in-out.
    inline float interpSineInOut(float x) {
        return -(std::cos(F_PI * x) - 1.0f) / 2.0f;
    }

    /// @brief Quadratic ease-in.
    constexpr float interpQuadIn(float x) {
        return x * x;
    }

    /// @brief Quadratic ease-out.
    constexpr float interpQuadOut(float x) {
        return 1.0f - (1.0f - x) * (1.0f - x);
    }

    /// @brief Quadratic ease-in-out.
    constexpr float interpQuadInOut(float x) {
        if (x < 0.5f) {
            return 2.0f * x * x;
        }
        float const f = (-2.0f * x) + 2.0f;
        return 1.0f - (f * f) / 2.0f;
    }

    /// @brief Cubic ease-in.
    constexpr float interpCubicIn(float x) {
        return x * x * x;
    }

    /// @brief Cubic ease-out.
    constexpr float interpCubicOut(float x) {
        float const f = 1.0f - x;
        return 1.0f - (f * f * f);
    }

    /// @brief Cubic ease-in-out.
    constexpr float interpCubicInOut(float x) {
        if (x < 0.5f) {
            return 4.0f * x * x * x;
        }
        float const f = (-2.0f * x) + 2.0f;
        return 1.0f - (f * f * f) / 2.0f;
    }

    /// @brief Quartic ease-in.
    constexpr float interpQuartIn(float x) {
        return x * x * x * x;
    }

    /// @brief Quartic ease-out.
    constexpr float interpQuartOut(float x) {
        float const f = 1.0f - x;
        return 1.0f - (f * f * f * f);
    }

    /// @brief Quartic ease-in-out.
    constexpr float interpQuartInOut(float x) {
        if (x < 0.5f) {
            return 8.0f * x * x * x * x;
        }
        float const f = (-2.0f * x) + 2.0f;
        return 1.0f - (f * f * f * f) / 2.0f;
    }

    /// @brief Quintic ease-in.
    constexpr float interpQuintIn(float x) {
        return x * x * x * x * x;
    }

    /// @brief Quintic ease-out.
    constexpr float interpQuintOut(float x) {
        float const f = 1.0f - x;
        return 1.0f - (f * f * f * f * f);
    }

    /// @brief Quintic ease-in-out.
    constexpr float interpQuintInOut(float x) {
        if (x < 0.5f) {
            return 16.0f * x * x * x * x * x;
        }
        float const f = (-2.0f * x) + 2.0f;
        return 1.0f - (f * f * f * f * f) / 2.0f;
    }

    /// @brief Exponential ease-in.
    inline float interpExpoIn(float x) {
        return x == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * x - 10.0f);
    }

    /// @brief Exponential ease-out.
    inline float interpExpoOut(float x) {
        return x == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * x);
    }

    /// @brief Exponential ease-in-out.
    inline float interpExpoInOut(float x) {
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
               : x < 0.5f ? std::pow(2.0f, 20.0f * x - 10.0f) / 2.0f
                          : (2.0f - std::pow(2.0f, -20.0f * x + 10.0f)) / 2.0f;
    }

    /// @brief Circular ease-in.
    inline float interpCircIn(float x) {
        return 1.0f - std::sqrt(1.0f - (x * x));
    }

    /// @brief Circular ease-out.
    inline float interpCircOut(float x) {
        float const f = x - 1.0f;
        return std::sqrt(1.0f - (f * f));
    }

    /// @brief Circular ease-in-out.
    inline float interpCircInOut(float x) {
        if (x < 0.5f) {
            float const f = 2.0f * x;
            return (1.0f - std::sqrt(1.0f - (f * f))) / 2.0f;
        }
        float const f = (-2.0f * x) + 2.0f;
        return (std::sqrt(1.0f - (f * f)) + 1.0f) / 2.0f;
    }

    /// @brief Back ease-in (overshoots slightly at the start).
    constexpr float interpBackIn(float x) {
        float const c1 = 1.70158f;
        float const c3 = c1 + 1.0f;
        return c3 * x * x * x - c1 * x * x;
    }

    /// @brief Back ease-out (overshoots slightly at the end).
    constexpr float interpBackOut(float x) {
        float const c1 = 1.70158f;
        float const c3 = c1 + 1.0f;
        float const f = x - 1.0f;
        return 1.0f + (c3 * f * f * f) + (c1 * f * f);
    }

    /// @brief Back ease-in-out.
    constexpr float interpBackInOut(float x) {
        float const c1 = 1.70158f;
        float const c2 = c1 * 1.525f;
        if (x < 0.5f) {
            float const f = 2.0f * x;
            return (f * f * ((c2 + 1.0f) * f - c2)) / 2.0f;
        }
        float const f = (2.0f * x) - 2.0f;
        return ((f * f * ((c2 + 1.0f) * f + c2)) + 2.0f) / 2.0f;
    }

    /// @brief Elastic ease-in.
    inline float interpElasticIn(float x) {
        float const c4 = (2.0f * F_PI) / 3.0f;
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
                   : -std::pow(2.0f, 10.0f * x - 10.0f) * std::sin((x * 10.0f - 10.75f) * c4);
    }

    /// @brief Elastic ease-out.
    inline float interpElasticOut(float x) {
        float const c4 = (2.0f * F_PI) / 3.0f;
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
                   : std::pow(2.0f, -10.0f * x) * std::sin((x * 10.0f - 0.75f) * c4) + 1.0f;
    }

    /// @brief Elastic ease-in-out.
    inline float interpElasticInOut(float x) {
        float const c5 = (2.0f * F_PI) / 4.5f;
        return x == 0.0f
                   ? 0.0f
               : x == 1.0f
                   ? 1.0f
               : x < 0.5f
                   ? -(std::pow(2.0f, 20.0f * x - 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) / 2.0f
                   : (std::pow(2.0f, -20.0f * x + 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) / 2.0f + 1.0f;
    }

    constexpr float interpBounceOut(float x);

    /// @brief Bounce ease-in.
    constexpr float interpBounceIn(float x) {
        return 1 - interpBounceOut(1.0f - x);
    }

    /// @brief Bounce ease-out.
    constexpr float interpBounceOut(float x) {
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
    constexpr float interpBounceInOut(float x) {
        return x < 0.5f
                   ? (1.0f - interpBounceOut(1.0f - 2.0f * x)) / 2.0f
                   : (1.0f + interpBounceOut(2.0f * x - 1.0f)) / 2.0f;
    }

    /// @brief Function pointer type for easing functions.
    typedef float (*InterpFunction)(float);

    /// @brief Array from InterpType to the corresponding easing function, indexed by enum value.
    inline constexpr std::array<InterpFunction, 34> InterpFuncs{{
        interpLinear,       // Unknown (invalid; placeholder so the array stays indexed)
        interpStep,         // Step
        interpLinear,       // Linear
        interpSmooth,       // Smooth
        interpSineIn,       // SineIn
        interpSineOut,      // SineOut
        interpSineInOut,    // SineInOut
        interpQuadIn,       // QuadIn
        interpQuadOut,      // QuadOut
        interpQuadInOut,    // QuadInOut
        interpCubicIn,      // CubicIn
        interpCubicOut,     // CubicOut
        interpCubicInOut,   // CubicInOut
        interpQuartIn,      // QuartIn
        interpQuartOut,     // QuartOut
        interpQuartInOut,   // QuartInOut
        interpQuintIn,      // QuintIn
        interpQuintOut,     // QuintOut
        interpQuintInOut,   // QuintInOut
        interpExpoIn,       // ExpoIn
        interpExpoOut,      // ExpoOut
        interpExpoInOut,    // ExpoInOut
        interpCircIn,       // CircIn
        interpCircOut,      // CircOut
        interpCircInOut,    // CircInOut
        interpBackIn,       // BackIn
        interpBackOut,      // BackOut
        interpBackInOut,    // BackInOut
        interpElasticIn,    // ElasticIn
        interpElasticOut,   // ElasticOut
        interpElasticInOut, // ElasticInOut
        interpBounceIn,     // BounceIn
        interpBounceOut,    // BounceOut
        interpBounceInOut,  // BounceInOut
    }};

    // If the enum grows, the array must stay in sync.
    static_assert(InterpFuncs.size() == static_cast<size_t>(InterpType::BounceInOut) + 1);

    /**
     * @brief Interpolates between two values using the specified easing curve.
     *
     * Usable in a constant expression when @p T is a literal type and @p type
     * names a polynomial curve. The sine, expo, circ and elastic families call
     * @c std::sin / @c std::pow / @c std::sqrt, which are not @c constexpr
     * before C++26, so those evaluate at runtime only.
     *
     * @param a    Start value (returned when @p t == 0).
     * @param b    End value (returned when @p t == 1).
     * @param t    Normalised position in [0, 1].
     * @param type Easing curve to apply.
     * @return Interpolated value.
     */
    template <typename T>
    constexpr T Interp(T a, T b, float t, InterpType type) {
        assert(type != InterpType::Unknown && "Unknown interp type should never be used.");
        auto const interpFunc = InterpFuncs[static_cast<size_t>(type)];
        return (t == 0.0f) ? a : (t == 1.0f) ? b : (a + (b - a) * interpFunc(t));
    }
}
// NOLINTEND
