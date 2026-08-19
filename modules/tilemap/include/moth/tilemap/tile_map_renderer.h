#pragma once

#include "moth/tilemap/tile_map.h"

#include "moth/graphics/graphics/igraphics.h"
#include "moth/graphics/graphics/image.h"

#include <functional>
#include <string>
#include <vector>

namespace moth::tilemap {
    /// @brief Resolves a tileset/tile image path to a loaded @c Image.
    ///
    /// Return an empty image for a path that cannot be loaded (the tile is then
    /// skipped). Implementations should cache by path — the resolver is invoked
    /// once per drawn tile.
    using TileImageResolver = std::function<moth::gfx::Image(std::string const& imagePath)>;

    /**
     * @brief Draws the visible tiles of @p map, culled to @p viewRect (map pixel space).
     *
     * The caller must apply the camera transform (@c IGraphics::SetTransform)
     * beforehand. @p tilesetImages[i] is the atlas image for @c map.tilesets[i];
     * tiles whose tileset image is empty are skipped. Tile and object layers are
     * drawn interleaved by their @c order field, so Tiled's original draw order
     * is preserved; object layers draw their tile objects (objects with a
     * non-empty @c MapObject::tile) and skip shape objects. Each layer's opacity
     * is applied via @c SetColor (the draw colour is reset to opaque white
     * afterwards). Horizontal/vertical/diagonal flips are honoured.
     *
     * This overload only handles atlas tilesets; use the @c TileImageResolver
     * overload to draw image-collection tilesets too.
     *
     * @p timeMs advances tile animations (Tiled semantics: all instances of an
     * animated tile share one phase); pass the accumulated game time in
     * milliseconds, or leave 0 for static maps.
     *
     * @p cameraPosition is the world-space view centre. When the map's layers
     * carry a non-identity parallax factor, each layer is offset by
     * @c (cameraPosition - parallaxOrigin) * (1 - parallax) so it scrolls at the
     * Tiled parallax speed; pass @c Camera::GetPosition() to enable parallax
     * (leave @c {} for maps without parallax).
     *
     * Infinite maps are culled at chunk granularity; finite maps at tile
     * granularity.
     */
    void DrawTileMap(moth::gfx::IGraphics& graphics,
                     TileMap const& map,
                     std::vector<moth::gfx::Image> const& tilesetImages,
                     FloatRect const& viewRect,
                     std::uint32_t timeMs = 0,
                     FloatVec2 const& cameraPosition = {});

    /**
     * @brief Draws the visible tiles of @p map, resolving each tile's image
     * through @p resolve.
     *
     * Identical to the atlas overload, but also supports image-collection
     * tilesets: for a tileset without a shared atlas image, @p resolve is called
     * with each tile's own image path (from @c Tileset::tileImages). The source
     * rectangle for such tiles is their own sub-rect rather than a computed grid
     * cell. @p resolve should return the image for @p imagePath, or an empty
     * image to skip the tile.
     */
    void DrawTileMap(moth::gfx::IGraphics& graphics,
                     TileMap const& map,
                     TileImageResolver const& resolve,
                     FloatRect const& viewRect,
                     std::uint32_t timeMs = 0,
                     FloatVec2 const& cameraPosition = {});
}
