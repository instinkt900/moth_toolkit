#include "moth_graphics/utils/polygon_triangulation.h"

#include <catch2/catch_all.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

using moth_graphics::FloatVec2;
using moth_graphics::graphics::TriangulatePolygon;
using moth_graphics::graphics::detail::TriangulatePolygonIndices;

namespace {
    float TriangleArea(FloatVec2 const& a, FloatVec2 const& b, FloatVec2 const& c) {
        return std::abs(((b.x - a.x) * (c.y - a.y)) - ((b.y - a.y) * (c.x - a.x))) * 0.5f;
    }

    float PolygonArea(std::vector<FloatVec2> const& poly) {
        float sum = 0.0f;
        size_t const n = poly.size();
        for (size_t i = 0; i < n; ++i) {
            FloatVec2 const& p0 = poly[i];
            FloatVec2 const& p1 = poly[(i + 1) % n];
            sum += (p0.x * p1.y) - (p1.x * p0.y);
        }
        return std::abs(sum) * 0.5f;
    }

    // Total area of a flat triangle-vertex list; equals the polygon's area only
    // when the triangulation covers the interior exactly and nothing outside.
    float TriangulatedArea(std::vector<FloatVec2> const& verts) {
        float total = 0.0f;
        for (size_t i = 0; i + 2 < verts.size(); i += 3) {
            total += TriangleArea(verts[i], verts[i + 1], verts[i + 2]);
        }
        return total;
    }
}

TEST_CASE("TriangulatePolygon rejects degenerate input", "[polygon]") {
    std::vector<FloatVec2> two{ { 0.0f, 0.0f }, { 1.0f, 0.0f } };
    CHECK(TriangulatePolygon(two.data(), two.size()).empty());
    CHECK(TriangulatePolygon(nullptr, 5).empty());
    CHECK(TriangulatePolygon(two.data(), 0).empty());
}

TEST_CASE("TriangulatePolygon triangulates a convex square", "[polygon]") {
    std::vector<FloatVec2> square{ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
    auto const verts = TriangulatePolygon(square.data(), square.size());

    REQUIRE(verts.size() == (square.size() - 2) * 3);
    CHECK(TriangulatedArea(verts) == Catch::Approx(PolygonArea(square)));
}

TEST_CASE("TriangulatePolygon handles a concave polygon", "[polygon]") {
    // An L-shape: the vertex (1,1) is a reflex corner.
    std::vector<FloatVec2> shape{
        { 0.0f, 0.0f }, { 2.0f, 0.0f }, { 2.0f, 1.0f },
        { 1.0f, 1.0f }, { 1.0f, 2.0f }, { 0.0f, 2.0f }
    };
    auto const verts = TriangulatePolygon(shape.data(), shape.size());

    REQUIRE(verts.size() == (shape.size() - 2) * 3);
    // Area matching proves no triangle bridges the concave notch.
    CHECK(TriangulatedArea(verts) == Catch::Approx(PolygonArea(shape)));
}

TEST_CASE("TriangulatePolygon is winding-agnostic", "[polygon]") {
    // The same L-shape wound clockwise triangulates to the same coverage.
    std::vector<FloatVec2> shape{
        { 0.0f, 2.0f }, { 1.0f, 2.0f }, { 1.0f, 1.0f },
        { 2.0f, 1.0f }, { 2.0f, 0.0f }, { 0.0f, 0.0f }
    };
    auto const verts = TriangulatePolygon(shape.data(), shape.size());

    REQUIRE(verts.size() == (shape.size() - 2) * 3);
    CHECK(TriangulatedArea(verts) == Catch::Approx(PolygonArea(shape)));
}

TEST_CASE("TriangulatePolygonIndices returns in-range index triples", "[polygon]") {
    std::vector<FloatVec2> shape{
        { 0.0f, 0.0f }, { 2.0f, 0.0f }, { 2.0f, 1.0f },
        { 1.0f, 1.0f }, { 1.0f, 2.0f }, { 0.0f, 2.0f }
    };
    auto const indices = TriangulatePolygonIndices(shape.data(), shape.size());

    REQUIRE(indices.size() == (shape.size() - 2) * 3);
    for (auto index : indices) {
        CHECK(index < shape.size());
    }
}
