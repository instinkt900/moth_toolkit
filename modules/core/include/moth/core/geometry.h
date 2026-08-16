#pragma once

#include "moth/core/aabb.h"
#include "moth/core/rect.h"
#include "moth/core/vector.h"
#include "moth/core/vector_utils.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace moth::core {
    /// @brief A circle defined by a centre point and radius.
    struct Circle {
        FloatVec2 center = { 0.0f, 0.0f };
        float radius = 0.0f;
    };

    /// @brief A line segment between two points.
    struct Segment {
        FloatVec2 start = { 0.0f, 0.0f };
        FloatVec2 end = { 0.0f, 0.0f };

        /// @brief Returns the end-to-start vector.
        FloatVec2 Direction() const { return end - start; }

        /// @brief Returns the segment length.
        float Length() const { return moth::core::Length(end - start); }

        /// @brief Returns the squared segment length.
        float LengthSq() const { return moth::core::LengthSq(end - start); }
    };

    /// @brief A ray: an origin plus a direction vector.
    ///
    /// @p direction should be normalized for @c t values (and hit distances) to
    /// be in world units.
    struct Ray {
        FloatVec2 origin = { 0.0f, 0.0f };
        FloatVec2 direction = { 1.0f, 0.0f };
    };

    /// @brief Returns @c true if @p point lies inside @p circle (inclusive).
    inline bool PointInCircle(FloatVec2 point, Circle circle) {
        return DistanceSq(point, circle.center) <= circle.radius * circle.radius;
    }

    /// @brief Returns @c true if two circles overlap (touching counts).
    inline bool Intersects(Circle a, Circle b) {
        float const sum = a.radius + b.radius;
        return DistanceSq(a.center, b.center) <= sum * sum;
    }

    /// @brief Returns @c true if @p circle overlaps @p box (touching counts).
    inline bool Intersects(Circle circle, AABB const& box) {
        FloatVec2 const closest = box.ClosestPoint(circle.center);
        return DistanceSq(circle.center, closest) <= circle.radius * circle.radius;
    }

    /// @brief Returns @c true if @p circle overlaps @p rect (touching counts).
    inline bool Intersects(Circle circle, FloatRect const& rect) {
        AABB const box = AABB::FromMinMax(rect.topLeft, rect.bottomRight);
        return Intersects(circle, box);
    }

    /// @brief Returns @c true if @p segment intersects @p circle.
    inline bool Intersects(Segment segment, Circle circle) {
        FloatVec2 const d = segment.end - segment.start;
        FloatVec2 const f = segment.start - circle.center;
        float const a = Dot(d, d);
        float const b = 2.0f * Dot(f, d);
        float const c = Dot(f, f) - (circle.radius * circle.radius);

        if (a < 1e-8f) {
            // Degenerate (zero-length) segment: treat as a point.
            return PointInCircle(segment.start, circle);
        }

        float const discriminant = (b * b) - (4.0f * a * c);
        if (discriminant < 0.0f) {
            return false;
        }
        float const root = std::sqrt(discriminant);
        float const t1 = (-b - root) / (2.0f * a);
        float const t2 = (-b + root) / (2.0f * a);
        return (t1 >= 0.0f && t1 <= 1.0f) || (t2 >= 0.0f && t2 <= 1.0f);
    }

    /// @brief Returns @c true if two segments intersect (touching counts).
    inline bool Intersects(Segment p, Segment q) {
        auto const cross = [](FloatVec2 const& a, FloatVec2 const& b) {
            return (a.x * b.y) - (a.y * b.x);
        };
        auto const orient = [&](FloatVec2 const& a, FloatVec2 const& b, FloatVec2 const& c) {
            return cross(b - a, c - a);
        };
        auto const onSegment = [](FloatVec2 const& point, Segment const& s) {
            return point.x >= std::min(s.start.x, s.end.x) && point.x <= std::max(s.start.x, s.end.x)
                && point.y >= std::min(s.start.y, s.end.y) && point.y <= std::max(s.start.y, s.end.y);
        };

        float const d1 = orient(q.start, q.end, p.start);
        float const d2 = orient(q.start, q.end, p.end);
        float const d3 = orient(p.start, p.end, q.start);
        float const d4 = orient(p.start, p.end, q.end);

        bool const straddles = ((d1 > 0.0f && d2 < 0.0f) || (d1 < 0.0f && d2 > 0.0f))
                            && ((d3 > 0.0f && d4 < 0.0f) || (d3 < 0.0f && d4 > 0.0f));
        if (straddles) {
            return true;
        }

        constexpr float kEpsilon = 1e-6f;
        if (std::abs(d1) <= kEpsilon && onSegment(p.start, q)) {
            return true;
        }
        if (std::abs(d2) <= kEpsilon && onSegment(p.end, q)) {
            return true;
        }
        if (std::abs(d3) <= kEpsilon && onSegment(q.start, p)) {
            return true;
        }
        if (std::abs(d4) <= kEpsilon && onSegment(q.end, p)) {
            return true;
        }
        return false;
    }

    /// @brief Casts @p ray against @p circle.
    ///
    /// @param[out] t      Distance along the ray to the first hit (if any).
    /// @param[out] normal Surface normal at the hit point (if any).
    /// @returns @c true on a hit. If the origin is inside the circle, reports the
    ///          exit hit.
    inline bool RaycastCircle(Ray ray, Circle circle, float* t = nullptr, FloatVec2* normal = nullptr) {
        FloatVec2 const m = ray.origin - circle.center;
        float const b = Dot(m, ray.direction);
        float const c = Dot(m, m) - (circle.radius * circle.radius);

        if (c > 0.0f && b > 0.0f) {
            return false; // origin outside and pointing away
        }
        float const discriminant = (b * b) - c;
        if (discriminant < 0.0f) {
            return false;
        }
        float const root = std::sqrt(discriminant);
        float hit = -b - root;
        if (hit < 0.0f) {
            hit = -b + root; // origin inside the circle: report the exit
        }
        if (hit < 0.0f) {
            return false;
        }
        if (t != nullptr) {
            *t = hit;
        }
        if (normal != nullptr) {
            FloatVec2 const point = ray.origin + (ray.direction * hit);
            FloatVec2 const n = point - circle.center;
            *normal = Normalized(n);
        }
        return true;
    }

    /// @brief Casts @p ray against an axis-aligned box (slab method).
    ///
    /// @param[out] t      Distance along the ray to the first hit (if any).
    /// @param[out] normal Surface normal at the hit point (if any).
    inline bool RaycastAABB(Ray ray, AABB box, float* t = nullptr, FloatVec2* normal = nullptr) {
        FloatVec2 const min = box.GetMin();
        FloatVec2 const max = box.GetMax();

        float tNear = -std::numeric_limits<float>::infinity();
        float tFar = std::numeric_limits<float>::infinity();
        FloatVec2 hitNormal{ 0.0f, 0.0f };

        for (int axis = 0; axis < 2; ++axis) {
            float const origin = axis == 0 ? ray.origin.x : ray.origin.y;
            float const direction = axis == 0 ? ray.direction.x : ray.direction.y;
            float const lo = axis == 0 ? min.x : min.y;
            float const hi = axis == 0 ? max.x : max.y;

            if (std::abs(direction) < 1e-8f) {
                if (origin < lo || origin > hi) {
                    return false; // parallel to this slab and outside it
                }
                continue;
            }

            float const inv = 1.0f / direction;
            float t1 = (lo - origin) * inv;
            float t2 = (hi - origin) * inv;
            float normalSign = -1.0f; // entered through the `lo` face
            if (t1 > t2) {
                std::swap(t1, t2);
                normalSign = 1.0f; // entered through the `hi` face
            }

            if (t1 > tNear) {
                tNear = t1;
                hitNormal = { 0.0f, 0.0f };
                if (axis == 0) {
                    hitNormal.x = normalSign;
                } else {
                    hitNormal.y = normalSign;
                }
            }
            tFar = std::min(tFar, t2);
            if (tNear > tFar) {
                return false;
            }
        }

        if (tNear < 0.0f || tFar < 0.0f) {
            return false;
        }
        if (t != nullptr) {
            *t = tNear;
        }
        if (normal != nullptr) {
            *normal = hitNormal;
        }
        return true;
    }

    /// @brief Casts @p ray against an axis-aligned rectangle.
    inline bool RaycastRect(Ray ray, FloatRect const& rect, float* t = nullptr, FloatVec2* normal = nullptr) {
        return RaycastAABB(ray, AABB::FromMinMax(rect.topLeft, rect.bottomRight), t, normal);
    }
}
