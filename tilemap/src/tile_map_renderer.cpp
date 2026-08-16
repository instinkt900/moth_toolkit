#include "moth/tilemap/tile_map_renderer.h"

#include "moth/core/color.h"
#include "moth/core/transform2d.h"

#include <algorithm>
#include <cmath>

namespace moth::tilemap {
    using moth::core::Color;
    using moth::core::Transform2D;
    using moth::gfx::graphics::IGraphics;
    using moth::gfx::graphics::Image;

    void DrawTileMap(IGraphics& graphics,
                     TileMap const& map,
                     std::vector<Image> const& tilesetImages,
                     FloatRect const& viewRect) {
        if (map.tileWidth <= 0 || map.tileHeight <= 0 || map.width <= 0 || map.height <= 0) {
            return;
        }

        // The range of tiles intersecting viewRect, clamped to the map bounds.
        int const minTileX = std::max(0, static_cast<int>(std::floor(viewRect.left() / static_cast<float>(map.tileWidth))));
        int const maxTileX = std::min(map.width - 1, static_cast<int>(std::ceil(viewRect.right() / static_cast<float>(map.tileWidth))) - 1);
        int const minTileY = std::max(0, static_cast<int>(std::floor(viewRect.top() / static_cast<float>(map.tileHeight))));
        int const maxTileY = std::min(map.height - 1, static_cast<int>(std::ceil(viewRect.bottom() / static_cast<float>(map.tileHeight))) - 1);

        if (minTileX > maxTileX || minTileY > maxTileY) {
            return;
        }

        for (auto const& layer : map.layers) {
            if (!layer.visible || layer.opacity <= 0.0f) {
                continue;
            }

            graphics.SetColor(Color{ 1.0f, 1.0f, 1.0f, layer.opacity });

            for (int ty = minTileY; ty <= maxTileY; ++ty) {
                for (int tx = minTileX; tx <= maxTileX; ++tx) {
                    TileId const tile = layer.GetTile(tx, ty);
                    if (tile.IsEmpty()) {
                        continue;
                    }

                    Tileset const* tileset = map.FindTileset(tile.id);
                    if (tileset == nullptr) {
                        continue;
                    }
                    std::size_t const tilesetIndex = static_cast<std::size_t>(tileset - map.tilesets.data());
                    if (tilesetIndex >= tilesetImages.size()) {
                        continue;
                    }

                    Image const& atlas = tilesetImages[tilesetIndex];
                    if (!atlas) {
                        continue;
                    }

                    IntRect const sourceRect = tileset->GetTileRect(tileset->LocalId(tile.id));
                    FloatVec2 const position = map.TileToWorld(tx, ty);

                    // Wrap the tile's atlas sub-region in an Image so the flip
                    // flags can be honoured by the transform-based draw call.
                    Image const tileImage(atlas.GetTexture(), sourceRect);
                    graphics.DrawImage(tileImage,
                                       Transform2D{ position, 0.0f, FloatVec2{ 1.0f, 1.0f } },
                                       FloatVec2{ 0.0f, 0.0f },
                                       tile.flipHorizontal,
                                       tile.flipVertical);
                }
            }
        }

        graphics.SetColor(Color{ 1.0f, 1.0f, 1.0f, 1.0f });
    }
}
