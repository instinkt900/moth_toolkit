#pragma once

#include "moth_graphics/utils/vector.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace moth_graphics::graphics {
    namespace detail {
        // Ear-clipping triangulation of a simple (non-self-intersecting) polygon
        // given by its perimeter points in either winding order. Convex and
        // concave shapes are supported; holes are not. Returns a flat list of
        // index triples into `points` (three indices per triangle), or an empty
        // list if the input has fewer than three points or is degenerate.
        //
        // The output winding is not guaranteed — 2D fills here are not back-face
        // culled, so callers only need the coverage, not a particular orientation.

        inline float PolygonCross(FloatVec2 const& a, FloatVec2 const& b, FloatVec2 const& c) {
            return ((b.x - a.x) * (c.y - a.y)) - ((b.y - a.y) * (c.x - a.x));
        }

        // Inside or on the CCW triangle a-b-c. The caller already skips the ear's
        // own three vertices, so any point reaching here is a genuine other vertex;
        // one lying on an edge (e.g. a reflex vertex grazed by the candidate
        // diagonal) must block the clip, else the ear bridges a concave notch.
        inline bool PointInsideTriangle(FloatVec2 const& p, FloatVec2 const& a,
                                        FloatVec2 const& b, FloatVec2 const& c) {
            return PolygonCross(a, b, p) >= 0.0f && PolygonCross(b, c, p) >= 0.0f &&
                   PolygonCross(c, a, p) >= 0.0f;
        }

        inline std::vector<uint16_t> TriangulatePolygonIndices(FloatVec2 const* points, size_t count) {
            std::vector<uint16_t> triangles;
            // Indices are emitted as uint16_t, so the largest point index
            // (count - 1) must stay representable; reject counts that would
            // otherwise wrap silently into wrong vertex references.
            if (points == nullptr || count < 3 ||
                count > static_cast<size_t>(std::numeric_limits<uint16_t>::max()) + 1) {
                return triangles;
            }

            // Signed area fixes the winding: iterate the working list so that it
            // is always counter-clockwise, which the convexity test assumes.
            float area = 0.0f;
            for (size_t i = 0; i < count; ++i) {
                FloatVec2 const& p0 = points[i];
                FloatVec2 const& p1 = points[(i + 1) % count];
                area += (p0.x * p1.y) - (p1.x * p0.y);
            }

            std::vector<int> remaining(count);
            for (size_t i = 0; i < count; ++i) {
                remaining[i] = area >= 0.0f ? static_cast<int>(i)
                                            : static_cast<int>(count - 1 - i);
            }

            triangles.reserve((count - 2) * 3);

            // Clip ears until a triangle is left. `guard` bounds the search so a
            // malformed (self-intersecting) polygon can't spin forever.
            size_t cursor = 0;
            int guard = static_cast<int>(2 * remaining.size());
            while (remaining.size() > 3 && guard > 0) {
                size_t const n = remaining.size();
                size_t const prev = (cursor + n - 1) % n;
                size_t const next = (cursor + 1) % n;
                FloatVec2 const& a = points[remaining[prev]];
                FloatVec2 const& b = points[remaining[cursor]];
                FloatVec2 const& c = points[remaining[next]];

                bool ear = PolygonCross(a, b, c) > 0.0f; // convex corner
                if (ear) {
                    for (size_t k = 0; k < n; ++k) {
                        if (k == prev || k == cursor || k == next) {
                            continue;
                        }
                        if (PointInsideTriangle(points[remaining[k]], a, b, c)) {
                            ear = false;
                            break;
                        }
                    }
                }

                if (ear) {
                    triangles.push_back(static_cast<uint16_t>(remaining[prev]));
                    triangles.push_back(static_cast<uint16_t>(remaining[cursor]));
                    triangles.push_back(static_cast<uint16_t>(remaining[next]));
                    remaining.erase(remaining.begin() + static_cast<std::ptrdiff_t>(cursor));
                    guard = static_cast<int>(2 * remaining.size());
                    if (cursor >= remaining.size()) {
                        cursor = 0;
                    }
                } else {
                    cursor = (cursor + 1) % remaining.size();
                    --guard;
                }
            }

            // A guard-limited exit (self-intersecting or otherwise malformed
            // input) leaves more than three vertices unclipped. Per the
            // contract that is degenerate, so discard the partial fan rather
            // than return stray triangles.
            if (remaining.size() != 3) {
                triangles.clear();
                return triangles;
            }
            triangles.push_back(static_cast<uint16_t>(remaining[0]));
            triangles.push_back(static_cast<uint16_t>(remaining[1]));
            triangles.push_back(static_cast<uint16_t>(remaining[2]));
            return triangles;
        }
    }

    // Triangulate a simple polygon perimeter into a flat list of triangle
    // vertices (three per triangle) ready to hand to @c IGraphics::DrawTrianglesF.
    // Convex and concave outlines are supported; holes are not; fewer than three
    // points yields an empty list.
    //
    // Triangulate once and cache the result for large static shapes (e.g. land
    // regions), then draw the cached vertices each frame — this avoids repeating
    // the ear-clip. The vertices are in the same space as @p points; the active
    // transform is applied at draw time.
    inline std::vector<FloatVec2> TriangulatePolygon(FloatVec2 const* points, size_t count) {
        auto const indices = detail::TriangulatePolygonIndices(points, count);
        std::vector<FloatVec2> vertices;
        vertices.reserve(indices.size());
        for (auto index : indices) {
            vertices.push_back(points[index]);
        }
        return vertices;
    }
}
