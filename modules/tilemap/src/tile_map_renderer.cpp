#include "moth/tilemap/tile_map_renderer.h"

#include "moth/core/color.h"
#include "moth/core/transform2d.h"

#include <algorithm>
#include <cmath>

namespace moth::tilemap {
    using moth::core::Color;
    using moth::core::Transform2D;
    using moth::gfx::IGraphics;
    using moth::gfx::Image;

    void DrawTileMap(IGraphics& graphics,
                     TileMap const& map,
                     std::vector<Image> const& tilesetImages,
                     FloatRect const& viewRect,
                     std::uint32_t timeMs) {
        if (map.tileWidth <= 0 || map.tileHeight <= 0) {
            return;
        }

        // Draws a single tile at tile coordinates (@p tx, @p ty), resolving its
        // animation frame and flip flags.
        auto const drawTile = [&](int tx, int ty, TileId const& tile) {
            if (tile.IsEmpty()) {
                return;
            }

            Tileset const* tileset = map.FindTileset(tile.id);
            if (tileset == nullptr) {
                return;
            }
            std::size_t const tilesetIndex = static_cast<std::size_t>(tileset - map.tilesets.data());
            if (tilesetIndex >= tilesetImages.size()) {
                return;
            }

            Image const& atlas = tilesetImages[tilesetIndex];
            if (!atlas) {
                return;
            }

            int const resolvedId = ResolveTileId(*tileset, tileset->LocalId(tile.id), timeMs);
            IntRect const sourceRect = tileset->GetTileRect(resolvedId);
            FloatVec2 const topLeft = map.TileToWorld(tx, ty);

            // Tiled's flip flags compose as (rotation, flipX, flipY): the
            // anti-diagonal flag is a 90-degree clockwise rotation about the
            // tile centre plus a horizontal flip, applied before the H/V
            // mirrors. Square tiles only for the diagonal flip.
            FloatVec2 position = topLeft;
            FloatVec2 pivot{ 0.0f, 0.0f };
            float rotation = 0.0f;
            bool flipX = tile.flipHorizontal;
            bool flipY = tile.flipVertical;

            if (tile.flipDiagonal) {
                position += FloatVec2{ static_cast<float>(map.tileWidth) * 0.5f,
                                       static_cast<float>(map.tileHeight) * 0.5f };
                pivot = FloatVec2{ 0.5f, 0.5f };
                rotation = 90.0f;
                flipX = !tile.flipHorizontal;
            }

            // Wrap the tile's atlas sub-region in an Image so the flip flags
            // can be honoured by the transform-based draw call.
            Image const tileImage(atlas.GetTexture(), sourceRect);
            graphics.DrawImage(tileImage,
                               Transform2D{ position, rotation, FloatVec2{ 1.0f, 1.0f } },
                               pivot,
                               flipX,
                               flipY);
        };

        for (auto const& layer : map.layers) {
            if (!layer.visible || layer.opacity <= 0.0f) {
                continue;
            }

            graphics.SetColor(Color{ 1.0f, 1.0f, 1.0f, layer.opacity });

            if (map.infinite) {
                // Cull at chunk granularity: draw only the chunks intersecting
                // viewRect.
                float const chunkWidth = static_cast<float>(kChunkSize * map.tileWidth);
                float const chunkHeight = static_cast<float>(kChunkSize * map.tileHeight);

                for (auto const& chunk : layer.chunks) {
                    float const cx0 = static_cast<float>(chunk.x) * chunkWidth;
                    float const cy0 = static_cast<float>(chunk.y) * chunkHeight;
                    if (cx0 + chunkWidth <= viewRect.left() || cx0 >= viewRect.right() ||
                        cy0 + chunkHeight <= viewRect.top() || cy0 >= viewRect.bottom()) {
                        continue;
                    }

                    int const baseTx = chunk.x * kChunkSize;
                    int const baseTy = chunk.y * kChunkSize;
                    for (int ly = 0; ly < kChunkSize; ++ly) {
                        for (int lx = 0; lx < kChunkSize; ++lx) {
                            drawTile(baseTx + lx, baseTy + ly, chunk.tiles[static_cast<std::size_t>(ly * kChunkSize + lx)]);
                        }
                    }
                }
            } else {
                // The range of tiles intersecting viewRect, clamped to the map bounds.
                int const minTileX = std::max(0, static_cast<int>(std::floor(viewRect.left() / static_cast<float>(map.tileWidth))));
                int const maxTileX = std::min(map.width - 1, static_cast<int>(std::ceil(viewRect.right() / static_cast<float>(map.tileWidth))) - 1);
                int const minTileY = std::max(0, static_cast<int>(std::floor(viewRect.top() / static_cast<float>(map.tileHeight))));
                int const maxTileY = std::min(map.height - 1, static_cast<int>(std::ceil(viewRect.bottom() / static_cast<float>(map.tileHeight))) - 1);

                if (minTileX > maxTileX || minTileY > maxTileY) {
                    continue;
                }

                for (int ty = minTileY; ty <= maxTileY; ++ty) {
                    for (int tx = minTileX; tx <= maxTileX; ++tx) {
                        drawTile(tx, ty, layer.GetTile(tx, ty));
                    }
                }
            }
        }

        graphics.SetColor(Color{ 1.0f, 1.0f, 1.0f, 1.0f });
    }
}
