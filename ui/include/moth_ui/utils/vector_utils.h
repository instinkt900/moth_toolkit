#pragma once

#include "moth_ui/utils/vector.h"
#include <cassert>
#include <cmath>

namespace moth_ui {
    /**
     * @brief Computes the dot product of two vectors.
     * @param a First vector.
     * @param b Second vector.
     * @return Scalar dot product.
     */
    template <typename T, int Dim>
    inline T Dot(Vector<T, Dim> const& a, Vector<T, Dim> const& b) {
        T sum = 0;
        for (int i = 0; i < Dim; ++i) {
            sum += a.data[i] * b.data[i];
        }
        return sum;
    }

    /**
     * @brief Returns the squared length (magnitude²) of a vector.
     * @param a Input vector.
     */
    template <typename T, int Dim>
    inline T LengthSq(Vector<T, Dim> const& a) {
        return Dot(a, a);
    }

    /**
     * @brief Returns the length (magnitude) of a vector.
     * @param a Input vector.
     */
    template <typename T, int Dim>
    inline T Length(Vector<T, Dim> const& a) {
        return std::sqrt(LengthSq(a));
    }

    /**
     * @brief Returns the squared Euclidean distance between two points.
     * @param a First point.
     * @param b Second point.
     */
    template <typename T, int Dim>
    inline T DistanceSq(Vector<T, Dim> const& a, Vector<T, Dim> const& b) {
        Vector<T, Dim> c = b - a;
        return LengthSq(c);
    }

    /**
     * @brief Returns the Euclidean distance between two points.
     * @param a First point.
     * @param b Second point.
     */
    template <typename T, int Dim>
    inline T Distance(Vector<T, Dim> const& a, Vector<T, Dim> const& b) {
        Vector<T, Dim> c = b - a;
        return Length(c);
    }

    /**
     * @brief Returns a unit-length copy of @p vec.
     * @param vec Input vector (must have non-zero length).
     * @return Unit vector in the same direction as @p vec.
     */
    template <typename T, int Dim>
    inline Vector<T, Dim> Normalized(Vector<T, Dim> const& vec) {
        assert(Length(vec) > 0);
        return vec / Length(vec);
    }

    /**
     * @brief Rotates a 2D vector by an angle in radians.
     * @param vec   Vector to rotate.
     * @param angle Counter-clockwise rotation angle in radians.
     * @return Rotated vector.
     */
    template <typename T>
    inline Vector<T, 2> Rotate2D(Vector<T, 2> const& vec, T angle) {
        Vector<T, 2> result;

        T cosTheta = std::cos(angle);
        T sinTheta = std::sin(angle);

        result.data[0] = cosTheta * vec.data[0] - sinTheta * vec.data[1];
        result.data[1] = sinTheta * vec.data[0] + cosTheta * vec.data[1];

        return result;
    }

    /**
     * @brief Translates a vector by an offset.
     * @param vec         Original vector.
     * @param translation Offset to add.
     * @return Translated vector.
     */
    template <typename T, int Dim>
    inline Vector<T, Dim> Translate(Vector<T, Dim> const& vec, Vector<T, Dim> translation) {
        return vec + translation;
    }

    /**
     * @brief Computes the signed angle (in radians) from vector @p a to vector @p b.
     * @param a First 2D vector.
     * @param b Second 2D vector.
     * @return Signed angle in radians in the range (−π, π].
     */
    template <typename T>
    inline T Angle(Vector<T, 2> const& a, Vector<T, 2> const& b) {
        return std::atan2((a.x * b.y) - (a.y * b.x), (a.x * b.x) + (a.y * b.y));
    }
}
