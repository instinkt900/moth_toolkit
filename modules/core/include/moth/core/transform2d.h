#pragma once

#include "moth/core/transform.h"
#include "moth/core/vector.h"

namespace moth::core {
    /**
     * @brief A lightweight 2D transform: translation, rotation, and scale.
     *
     * Rotation is in degrees, clockwise (matching @c FloatMat4x4::Rotation).
     * This is the ergonomic sibling of @c FloatMat4x4 for sprite-style drawing
     * (position + rotation + scale), without carrying a 4x4 matrix around.
     */
    struct Transform2D {
        FloatVec2 position = { 0.0f, 0.0f }; ///< Translation in world units.
        float rotation = 0.0f;               ///< Rotation in degrees, clockwise.
        FloatVec2 scale = { 1.0f, 1.0f };    ///< Scale factor (1 = no scaling).

        /// @brief Returns the identity transform.
        static Transform2D Identity() { return {}; }

        /// @brief Builds the equivalent 4x4 matrix: world = matrix * local.
        ///
        /// Points are scaled about the origin, rotated about the origin, then
        /// translated by @c position.
        FloatMat4x4 ToMatrix() const {
            return FloatMat4x4::Translation(position)
                 * FloatMat4x4::Rotation(rotation, { 0.0f, 0.0f })
                 * FloatMat4x4::Scale(scale);
        }

        /// @brief Builds the 4x4 matrix, scaling/rotating around @p pivot (local space).
        ///
        /// The point at @p pivot lands exactly on @c position, with scaling and
        /// rotation applied about that pivot.
        FloatMat4x4 ToMatrix(FloatVec2 pivot) const {
            return FloatMat4x4::Translation(position)
                 * FloatMat4x4::Rotation(rotation, { 0.0f, 0.0f })
                 * FloatMat4x4::Scale(scale)
                 * FloatMat4x4::Translation(FloatVec2{ -pivot.x, -pivot.y });
        }

        /// @brief Applies the transform (about the origin) to a 2D point.
        FloatVec2 TransformPoint(FloatVec2 point) const {
            return ToMatrix().TransformPoint(point);
        }
    };
}
