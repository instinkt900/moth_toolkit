#pragma once

#include "moth/core/vector.h"
#include "moth/core/vector_utils.h"

#include <algorithm>
#include <cmath>

namespace moth::core {
    /**
     * @brief An axis-aligned bounding box defined by a centre and half-extents.
     *
     * A game-oriented complement to @c Rect (which stores min/max corners).
     * Provides containment, overlap, closest-point, distance, and merge queries
     * used by culling and collision code.
     */
    struct AABB {
        FloatVec2 center = { 0.0f, 0.0f };     ///< Centre of the box.
        FloatVec2 halfExtents = { 0.0f, 0.0f }; ///< Half width/height (non-negative).

        AABB() = default;

        /// @brief Constructs from a centre and half-extents.
        AABB(FloatVec2 center, FloatVec2 halfExtents)
            : center(center)
            , halfExtents(halfExtents) {}

        /// @brief Constructs from min/max corners.
        static AABB FromMinMax(FloatVec2 min, FloatVec2 max) {
            return { (min + max) * 0.5f, (max - min) * 0.5f };
        }

        /// @brief Returns the minimum (top-left) corner.
        FloatVec2 GetMin() const { return center - halfExtents; }

        /// @brief Returns the maximum (bottom-right) corner.
        FloatVec2 GetMax() const { return center + halfExtents; }

        /// @brief Returns the full width/height.
        FloatVec2 GetSize() const { return halfExtents * 2.0f; }

        /// @brief Returns @c true if @p point lies inside the box (inclusive).
        bool Contains(FloatVec2 point) const {
            auto const d = point - center;
            return std::abs(d.x) <= halfExtents.x && std::abs(d.y) <= halfExtents.y;
        }

        /// @brief Returns @c true if @p other lies entirely inside this box.
        bool Contains(AABB const& other) const {
            return Contains(other.GetMin()) && Contains(other.GetMax());
        }

        /// @brief Returns @c true if this box overlaps @p other (touching counts).
        bool Overlaps(AABB const& other) const {
            auto const d = other.center - center;
            return std::abs(d.x) <= (halfExtents.x + other.halfExtents.x)
                && std::abs(d.y) <= (halfExtents.y + other.halfExtents.y);
        }

        /// @brief Returns the point on (or inside) the box nearest to @p point.
        FloatVec2 ClosestPoint(FloatVec2 point) const {
            auto const min = GetMin();
            auto const max = GetMax();
            return {
                std::clamp(point.x, min.x, max.x),
                std::clamp(point.y, min.y, max.y),
            };
        }

        /// @brief Returns the squared distance from @p point to the box (0 if inside).
        float DistanceSq(FloatVec2 point) const {
            return LengthSq(point - ClosestPoint(point));
        }

        /// @brief Returns the distance from @p point to the box (0 if inside).
        float Distance(FloatVec2 point) const {
            return std::sqrt(DistanceSq(point));
        }

        /// @brief Returns the smallest box enclosing both this box and @p other.
        AABB Merged(AABB const& other) const {
            auto const min = GetMin();
            auto const max = GetMax();
            auto const otherMin = other.GetMin();
            auto const otherMax = other.GetMax();
            return FromMinMax(
                FloatVec2{ std::min(min.x, otherMin.x), std::min(min.y, otherMin.y) },
                FloatVec2{ std::max(max.x, otherMax.x), std::max(max.y, otherMax.y) });
        }

        /// @brief Grows the box by @p amount in each direction.
        void Expand(FloatVec2 amount) {
            halfExtents += amount;
        }

        /// @brief Returns a copy grown by @p amount in each direction.
        AABB Expanded(FloatVec2 amount) const {
            AABB result = *this;
            result.Expand(amount);
            return result;
        }
    };

    /// @brief Returns @c true if the centre and half-extents are identical.
    inline bool operator==(AABB const& a, AABB const& b) {
        return a.center == b.center && a.halfExtents == b.halfExtents;
    }

    /// @brief Returns @c true if the boxes differ.
    inline bool operator!=(AABB const& a, AABB const& b) {
        return !(a == b);
    }
}
