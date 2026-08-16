#pragma once

#include "moth/core/transform.h"

#include <cmath>

namespace moth::core {
    namespace {
        constexpr float kPi = 3.14159265f;
        constexpr float kTwoPi = 2.0f * kPi;
    }

    /// @brief Wraps an angle in degrees to the range [-180, 180].
    inline float WrapAngleDegrees(float degrees) {
        degrees = std::fmod(degrees + 180.0f, 360.0f);
        if (degrees < 0.0f) {
            degrees += 360.0f;
        }
        return degrees - 180.0f;
    }

    /// @brief Wraps an angle in radians to the range [-π, π].
    inline float WrapAngleRadians(float radians) {
        radians = std::fmod(radians + kPi, kTwoPi);
        if (radians < 0.0f) {
            radians += kTwoPi;
        }
        return radians - kPi;
    }

    /// @brief Returns the shortest signed difference (degrees) from @p from to @p to, in [-180, 180].
    inline float AngleDeltaDegrees(float from, float to) {
        return WrapAngleDegrees(to - from);
    }

    /// @brief Returns the shortest signed difference (radians) from @p from to @p to, in [-π, π].
    inline float AngleDeltaRadians(float from, float to) {
        return WrapAngleRadians(to - from);
    }

    /// @brief Linearly interpolates two angles (degrees) along the shortest arc.
    inline float LerpAngleDegrees(float from, float to, float t) {
        return from + (AngleDeltaDegrees(from, to) * t);
    }

    /// @brief Linearly interpolates two angles (radians) along the shortest arc.
    inline float LerpAngleRadians(float from, float to, float t) {
        return from + (AngleDeltaRadians(from, to) * t);
    }
}
