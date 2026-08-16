#pragma once

#include "moth_graphics/utils/vector.h"

#include <cstddef>
#include <vector>

namespace moth::gfx::graphics {
    // Triangulate a simple polygon perimeter into a flat list of triangle
    // vertices (three per triangle) ready to hand to @c IGraphics::DrawTrianglesF.
    // Convex and concave outlines are supported; holes are not; fewer than three
    // points yields an empty list.
    //
    // Triangulate once and cache the result for large static shapes (e.g. land
    // regions), then draw the cached vertices each frame — this avoids repeating
    // the ear-clip. The vertices are in the same space as @p points; the active
    // transform is applied at draw time.
    std::vector<FloatVec2> TriangulatePolygon(FloatVec2 const* points, size_t count);
}
