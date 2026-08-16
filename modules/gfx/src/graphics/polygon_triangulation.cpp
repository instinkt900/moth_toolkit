#include "polygon_triangulation_detail.h"

#include "moth_graphics/utils/polygon_triangulation.h"

namespace moth::gfx::graphics {
    std::vector<FloatVec2> TriangulatePolygon(FloatVec2 const* points, size_t count) {
        auto const indices = detail::TriangulatePolygonIndices(points, count);
        std::vector<FloatVec2> vertices;
        vertices.reserve(indices.size());
        for (auto index : indices) {
            vertices.push_back(points[index]);
        }
        return vertices;
    }
}
