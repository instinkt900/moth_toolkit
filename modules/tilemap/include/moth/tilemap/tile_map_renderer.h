#pragma once

#include "moth/tilemap/tile_map.h"

#include "moth/graphics/graphics/igraphics.h"
#include "moth/graphics/graphics/image.h"

#include <vector>

namespace moth::tilemap {
    /**
     * @brief Draws the visible tiles of @p map, culled to @p viewRect (map pixel space).
     *
     * The caller must apply the camera transform (@c IGraphics::SetTransform)
     * beforehand. @p tilesetImages[i] is the atlas image for @c map.tilesets[i];
     * tiles whose tileset image is empty are skipped. Layer order is preserved
     * (bottom-up) and each layer's opacity is applied via @c SetColor (the draw
     * colour is reset to opaque white afterwards). Horizontal/vertical/diagonal
     * flips are honoured.
     *
     * @p timeMs advances tile animations (Tiled semantics: all instances of an
     * animated tile share one phase); pass the accumulated game time in
     * milliseconds, or leave 0 for static maps.
     *
     * Infinite maps are culled at chunk granularity; finite maps at tile
     * granularity.
     */
    void DrawTileMap(moth::gfx::IGraphics& graphics,
                     TileMap const& map,
                     std::vector<moth::gfx::Image> const& tilesetImages,
                     FloatRect const& viewRect,
                     std::uint32_t timeMs = 0);
}
