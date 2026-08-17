#pragma once

#include "moth/graphics/graphics/color.h"
#include "moth/graphics/utils/vector.h"

namespace moth::gfx {
    /// @brief A textured vertex with per-vertex position, UV and color.
    ///
    /// Used with @c IGraphics::DrawTexturedTrianglesF. The UV is in texture
    /// space (0..1 across the full texture, or matching an atlas sub-region).
    struct TexturedVertex {
        FloatVec2 position; ///< Local (pre-transform) position in logical pixels.
        FloatVec2 uv;       ///< Texture-space UV coordinate.
        Color color;        ///< Per-vertex color (multiplied with the texture sample).
    };
}
