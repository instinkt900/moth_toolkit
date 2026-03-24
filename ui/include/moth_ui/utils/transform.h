#pragma once

#include "moth_ui/utils/vector.h"

#include <array>
#include <cmath>

namespace moth_ui {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    /// @brief The default rotation pivot: the centre of a node's bounds.
    inline FloatVec2 const kDefaultPivot = { 0.5f, 0.5f };
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

    /**
     * @brief A standard 4×4 row-major transform matrix.
     *
     * Row i, column j is accessed as m[i][j].
     * Transforms a 2D point (x, y) as (z=0, w=1):
     *   x' = m[0][0]*x + m[0][1]*y + m[0][3]
     *   y' = m[1][0]*x + m[1][1]*y + m[1][3]
     *
     * The identity transform leaves points unchanged.
     * Composition (operator*) applies the right-hand operand first.
     */
    struct FloatMat4x4 {
        std::array<std::array<float, 4>, 4> m = {{
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        }};

        /// @brief Returns the identity transform.
        static FloatMat4x4 Identity() { return {}; }

        /// @brief Returns a pure XY translation transform.
        static FloatMat4x4 Translation(FloatVec2 offset) {
            FloatMat4x4 t;
            t.m[0][3] = offset.x;
            t.m[1][3] = offset.y;
            return t;
        }

        /// @brief Returns a clockwise rotation around Z in degrees, pivoting around @p pivot (in the same space as the points being transformed).
        static FloatMat4x4 Rotation(float degrees, FloatVec2 pivot) {
            static constexpr float kDegToRad = 3.14159265358979f / 180.0f;
            float const rad = degrees * kDegToRad;
            float const cosA = std::cos(rad);
            float const sinA = std::sin(rad);
            FloatMat4x4 r;
            r.m[0][0] =  cosA;  r.m[0][1] = -sinA;  r.m[0][3] = (pivot.x * (1.0f - cosA)) + (pivot.y * sinA);
            r.m[1][0] =  sinA;  r.m[1][1] =  cosA;  r.m[1][3] = (pivot.y * (1.0f - cosA)) - (pivot.x * sinA);
            return r;
        }

        /// @brief Composes two transforms: applies @p rhs first, then @c *this.
        FloatMat4x4 operator*(FloatMat4x4 const& rhs) const {
            FloatMat4x4 result;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    result.m[i][j] = 0.0f;
                    for (int k = 0; k < 4; ++k) {
                        result.m[i][j] += m[i][k] * rhs.m[k][j];
                    }
                }
            }
            return result;
        }

        /// @brief Applies the transform to a 2D point (z=0, w=1).
        FloatVec2 TransformPoint(FloatVec2 const& p) const {
            return {
                (m[0][0] * p.x) + (m[0][1] * p.y) + m[0][3],
                (m[1][0] * p.x) + (m[1][1] * p.y) + m[1][3],
            };
        }

        /// @brief Extracts the clockwise Z-rotation angle in degrees encoded in the matrix.
        float GetRotationDegrees() const {
            static constexpr float kRadToDeg = 180.0f / 3.14159265358979f;
            return std::atan2(m[1][0], m[0][0]) * kRadToDeg;
        }

        /**
         * @brief Returns the inverse of this transform.
         * @note Valid only for rigid-body transforms (rotation + translation, no scale or shear).
         *       For such transforms the inverse is R^T with translation -(R^T * t).
         */
        FloatMat4x4 Invert() const {
            FloatMat4x4 result;
            // Transpose the rotation sub-matrix
            result.m[0][0] = m[0][0];  result.m[0][1] = m[1][0];
            result.m[1][0] = m[0][1];  result.m[1][1] = m[1][1];
            // Inverse translation: -(R^T * t)
            result.m[0][3] = -((m[0][0] * m[0][3]) + (m[1][0] * m[1][3]));
            result.m[1][3] = -((m[0][1] * m[0][3]) + (m[1][1] * m[1][3]));
            return result;
        }
    };
}
