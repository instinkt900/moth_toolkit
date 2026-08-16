#pragma once

#include "moth/tilemap/tile_map.h"

#include "moth_graphics/graphics/igraphics.h"
#include "moth_graphics/graphics/image.h"

#include <vector>

namespace moth::tilemap {
    /**
     * @brief Draws the visible tiles of @p map, culled to @p viewRect (map pixel space).
     *
     * The caller must apply the camera transform (@c IGraphics::SetTransform)
     * beforehand. @p tilesetImages[i] is the atlas image for @c map.tilesets[i];
     * tiles whose tileset image is empty are skipped. Layer order is preserved
     * (bottom-up) and each layer's opacity is applied via @c SetColor (the draw
     * colour is reset to opaque white afterwards). Horizontal/vertical flips are
     * honoured; the diagonal flip is unpacked but not yet rendered.
     */
    void DrawTileMap(moth::gfx::graphics::IGraphics& graphics,
                     TileMap const& map,
                     std::vector<moth::gfx::graphics::Image> const& tilesetImages,
                     FloatRect const& viewRect);
}
