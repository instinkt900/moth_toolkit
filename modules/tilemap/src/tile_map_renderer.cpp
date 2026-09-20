#include "moth/tilemap/tile_map_renderer.h"

#include "moth/core/angle.h"
#include "moth/core/color.h"
#include "moth/core/transform2d.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

namespace moth::tilemap {
    using moth::core::Color;
    using moth::core::Transform2D;
    using moth::gfx::IGraphics;
    using moth::gfx::Image;

    namespace {
        // Draws the visible tiles of @p map, culled to @p viewRect. For each
        // tile, @p resolveTileImage returns the image to draw (given the owning
        // tileset and the animation-resolved local id), or an empty image to
        // skip the tile. Tile and object layers are interleaved by their @c order
        // field so the original Tiled draw order is preserved. Layers with a
        // non-identity parallax factor are offset by the layer's parallax.
        void DrawTileMapImpl(IGraphics& graphics,
                             TileMap const& map,
                             FloatRect const& viewRect,
                             std::uint32_t timeMs,
                             FloatVec2 const& cameraPosition,
                             std::function<Image(Tileset const&, int)> const& resolveTileImage,
                             TileImageResolver const& resolveLayerImage) {
            if (map.tileWidth <= 0 || map.tileHeight <= 0) {
                return;
            }

            // Tiled parallax: a layer with factor @p factor scrolls at that
            // fraction of the camera speed, pinned to the map's parallax origin.
            // Applied as a per-layer world-space offset (constant across the layer).
            auto const parallaxOffset = [&](FloatVec2 const& factor) {
                return (cameraPosition - map.parallaxOrigin) * (FloatVec2{ 1.0f, 1.0f } - factor);
            };

            // Draws a single tile image at @p topLeft (map pixel space), applying
            // the tile's flip flags plus an optional base rotation (object rotation)
            // and scale (resized tile objects). Rotation pivots on the tile centre.
            auto const drawTileImage = [&](Tileset const& tileset,
                                           int resolvedId,
                                           TileId const& tile,
                                           FloatVec2 const& topLeft,
                                           float baseRotation,
                                           FloatVec2 const& scale) {
                Image const image = resolveTileImage(tileset, resolvedId);
                if (!image) {
                    return;
                }

                IntRect const sourceRect = tileset.GetTileRect(resolvedId);
                float const tileW = static_cast<float>(sourceRect.w());
                float const tileH = static_cast<float>(sourceRect.h());

                // Tiled's flip flags compose as (rotation, flipX, flipY): the
                // anti-diagonal flag is a 90-degree clockwise rotation about the
                // tile centre plus a horizontal flip, applied before the H/V
                // mirrors. Square tiles only for the diagonal flip.
                FloatVec2 position = topLeft;
                FloatVec2 pivot{ 0.0f, 0.0f };
                float rotation = baseRotation;
                bool flipX = tile.flipHorizontal;
                bool flipY = tile.flipVertical;

                bool const diagonal = tile.flipDiagonal;
                if (diagonal || baseRotation != 0.0f) {
                    position += FloatVec2{ tileW * scale.x * 0.5f, tileH * scale.y * 0.5f };
                    pivot = FloatVec2{ 0.5f, 0.5f };
                }
                if (diagonal) {
                    rotation += moth::core::DegToRad(90.0f);
                    flipX = !tile.flipHorizontal;
                }

                // Wrap the tile's atlas sub-region in an Image so the flip flags
                // can be honoured by the transform-based draw call.
                Image const tileImage(image.GetTexture(), sourceRect);
                graphics.DrawImage(tileImage,
                                   Transform2D{ position, rotation, scale },
                                   pivot,
                                   flipX,
                                   flipY);
            };

            auto const drawTile = [&](int tx, int ty, TileId const& tile, FloatVec2 const& offset) {
                if (tile.IsEmpty()) {
                    return;
                }
                Tileset const* tileset = map.FindTileset(tile.id);
                if (tileset == nullptr) {
                    return;
                }
                int const resolvedId = ResolveTileId(*tileset, tileset->LocalId(tile.id), timeMs);
                drawTileImage(*tileset, resolvedId, tile, map.TileToWorld(tx, ty) + offset, 0.0f, FloatVec2{ 1.0f, 1.0f });
            };

            auto const drawTileLayer = [&](Layer const& layer, FloatVec2 const& offset) {
                FloatRect const layerView = viewRect - offset;

                if (map.infinite) {
                    // Cull at chunk granularity: draw only the chunks intersecting
                    // layerView.
                    float const chunkWidth = static_cast<float>(kChunkSize * map.tileWidth);
                    float const chunkHeight = static_cast<float>(kChunkSize * map.tileHeight);

                    for (auto const& chunk : layer.chunks) {
                        float const cx0 = static_cast<float>(chunk.x) * chunkWidth;
                        float const cy0 = static_cast<float>(chunk.y) * chunkHeight;
                        if (cx0 + chunkWidth <= layerView.left() || cx0 >= layerView.right() ||
                            cy0 + chunkHeight <= layerView.top() || cy0 >= layerView.bottom()) {
                            continue;
                        }

                        int const baseTx = chunk.x * kChunkSize;
                        int const baseTy = chunk.y * kChunkSize;
                        for (int ly = 0; ly < kChunkSize; ++ly) {
                            for (int lx = 0; lx < kChunkSize; ++lx) {
                                drawTile(baseTx + lx, baseTy + ly, chunk.tiles[static_cast<std::size_t>(ly * kChunkSize + lx)], offset);
                            }
                        }
                    }
                } else {
                    // The range of tiles intersecting layerView, clamped to the map bounds.
                    int const minTileX = std::max(0, static_cast<int>(std::floor(layerView.left() / static_cast<float>(map.tileWidth))));
                    int const maxTileX = std::min(map.width - 1, static_cast<int>(std::ceil(layerView.right() / static_cast<float>(map.tileWidth))) - 1);
                    int const minTileY = std::max(0, static_cast<int>(std::floor(layerView.top() / static_cast<float>(map.tileHeight))));
                    int const maxTileY = std::min(map.height - 1, static_cast<int>(std::ceil(layerView.bottom() / static_cast<float>(map.tileHeight))) - 1);

                    if (minTileX > maxTileX || minTileY > maxTileY) {
                        return;
                    }

                    for (int ty = minTileY; ty <= maxTileY; ++ty) {
                        for (int tx = minTileX; tx <= maxTileX; ++tx) {
                            drawTile(tx, ty, layer.GetTile(tx, ty), offset);
                        }
                    }
                }
            };

            auto const drawObjectLayer = [&](ObjectLayer const& objectLayer, FloatVec2 const& offset) {
                for (auto const& object : objectLayer.objects) {
                    if (!object.visible || object.tile.IsEmpty()) {
                        continue;
                    }
                    Tileset const* tileset = map.FindTileset(object.tile.id);
                    if (tileset == nullptr) {
                        continue;
                    }

                    int const resolvedId = ResolveTileId(*tileset, tileset->LocalId(object.tile.id), timeMs);
                    IntRect const sourceRect = tileset->GetTileRect(resolvedId);

                    // A tile object may be resized; scale the tile to the object's
                    // size (fall back to the tile's natural size when the object
                    // has no size).
                    FloatVec2 scale{ 1.0f, 1.0f };
                    if (object.size.x > 0.0f && sourceRect.w() > 0) {
                        scale.x = object.size.x / static_cast<float>(sourceRect.w());
                    }
                    if (object.size.y > 0.0f && sourceRect.h() > 0) {
                        scale.y = object.size.y / static_cast<float>(sourceRect.h());
                    }

                    // Tiled anchors a tile object at its bottom-left corner, where a plain
                    // rectangle object is anchored top-left. Shift up by the drawn height so
                    // the tile lands where the editor shows it.
                    FloatVec2 topLeft = object.position + offset;
                    topLeft.y -= static_cast<float>(sourceRect.h()) * scale.y;

                    drawTileImage(*tileset, resolvedId, object.tile, topLeft, object.rotation, scale);
                }
            };

            // Draws an image layer at its natural size. A repeating axis tiles
            // the image across the view, aligned to the layer's position; a
            // non-repeating axis draws the single copy only if it is visible.
            auto const drawImageLayer = [&](ImageLayer const& imageLayer, FloatVec2 const& offset) {
                if (imageLayer.imagePath.empty()) {
                    return;
                }
                Image const image = resolveLayerImage(imageLayer.imagePath);
                if (!image || image.GetWidth() <= 0 || image.GetHeight() <= 0) {
                    return;
                }

                float const imageW = static_cast<float>(image.GetWidth());
                float const imageH = static_cast<float>(image.GetHeight());
                FloatVec2 const origin = imageLayer.offset + offset;

                int minX = 0;
                int maxX = 0;
                if (imageLayer.repeatX) {
                    minX = static_cast<int>(std::floor((viewRect.left() - origin.x) / imageW));
                    maxX = static_cast<int>(std::ceil((viewRect.right() - origin.x) / imageW)) - 1;
                } else if (origin.x + imageW <= viewRect.left() || origin.x >= viewRect.right()) {
                    return;
                }

                int minY = 0;
                int maxY = 0;
                if (imageLayer.repeatY) {
                    minY = static_cast<int>(std::floor((viewRect.top() - origin.y) / imageH));
                    maxY = static_cast<int>(std::ceil((viewRect.bottom() - origin.y) / imageH)) - 1;
                } else if (origin.y + imageH <= viewRect.top() || origin.y >= viewRect.bottom()) {
                    return;
                }

                for (int iy = minY; iy <= maxY; ++iy) {
                    for (int ix = minX; ix <= maxX; ++ix) {
                        FloatVec2 const position{ origin.x + static_cast<float>(ix) * imageW,
                                                  origin.y + static_cast<float>(iy) * imageH };
                        graphics.DrawImage(image, Transform2D{ position, 0.0f, FloatVec2{ 1.0f, 1.0f } }, FloatVec2{ 0.0f, 0.0f });
                    }
                }
            };

            // Merge tile, object and image layers into a single draw order
            // (stable sort keeps layers sharing an @c order in their vector order).
            enum class LayerKind { Tile, Object, Image };
            struct DrawEntry {
                int order;
                LayerKind kind;
                std::size_t index;
            };
            std::vector<DrawEntry> drawOrder;
            drawOrder.reserve(map.layers.size() + map.objectLayers.size() + map.imageLayers.size());
            for (std::size_t i = 0; i < map.layers.size(); ++i) {
                drawOrder.push_back({ map.layers[i].order, LayerKind::Tile, i });
            }
            for (std::size_t i = 0; i < map.objectLayers.size(); ++i) {
                drawOrder.push_back({ map.objectLayers[i].order, LayerKind::Object, i });
            }
            for (std::size_t i = 0; i < map.imageLayers.size(); ++i) {
                drawOrder.push_back({ map.imageLayers[i].order, LayerKind::Image, i });
            }
            std::stable_sort(drawOrder.begin(), drawOrder.end(),
                             [](DrawEntry const& a, DrawEntry const& b) { return a.order < b.order; });

            // Applies a layer's tint colour and opacity as the draw colour. The
            // tint modulates the tile RGB; opacity multiplies the tint's alpha.
            auto const applyLayerColor = [&](Color const& tint, float opacity) {
                graphics.SetColor(Color{ tint.r, tint.g, tint.b, opacity * tint.a });
            };

            for (auto const& entry : drawOrder) {
                if (entry.kind == LayerKind::Object) {
                    auto const& objectLayer = map.objectLayers[entry.index];
                    if (!objectLayer.visible || objectLayer.opacity <= 0.0f) {
                        continue;
                    }
                    applyLayerColor(objectLayer.tint, objectLayer.opacity);
                    drawObjectLayer(objectLayer, parallaxOffset(objectLayer.parallax));
                } else if (entry.kind == LayerKind::Image) {
                    auto const& imageLayer = map.imageLayers[entry.index];
                    if (!imageLayer.visible || imageLayer.opacity <= 0.0f) {
                        continue;
                    }
                    applyLayerColor(imageLayer.tint, imageLayer.opacity);
                    drawImageLayer(imageLayer, parallaxOffset(imageLayer.parallax));
                } else {
                    auto const& layer = map.layers[entry.index];
                    if (!layer.visible || layer.opacity <= 0.0f) {
                        continue;
                    }
                    applyLayerColor(layer.tint, layer.opacity);
                    drawTileLayer(layer, parallaxOffset(layer.parallax));
                }
            }

            graphics.SetColor(Color{ 1.0f, 1.0f, 1.0f, 1.0f });
        }
    }

    void DrawTileMap(IGraphics& graphics,
                     TileMap const& map,
                     std::vector<Image> const& tilesetImages,
                     FloatRect const& viewRect,
                     std::uint32_t timeMs,
                     FloatVec2 const& cameraPosition) {
        DrawTileMapImpl(graphics, map, viewRect, timeMs, cameraPosition, [&](Tileset const& tileset, int) {
            std::size_t const index = static_cast<std::size_t>(&tileset - map.tilesets.data());
            if (index >= tilesetImages.size()) {
                return Image{};
            }
            return tilesetImages[index];
        }, [](std::string const&) { return Image{}; });
    }

    void DrawTileMap(IGraphics& graphics,
                     TileMap const& map,
                     TileImageResolver const& resolve,
                     FloatRect const& viewRect,
                     std::uint32_t timeMs,
                     FloatVec2 const& cameraPosition) {
        // Resolve each atlas tileset's image once per draw rather than once per tile.
        std::vector<Image> atlasImages;
        atlasImages.reserve(map.tilesets.size());
        for (auto const& tileset : map.tilesets) {
            atlasImages.push_back(tileset.IsImageCollection() ? Image{} : resolve(tileset.imagePath));
        }

        DrawTileMapImpl(graphics, map, viewRect, timeMs, cameraPosition, [&](Tileset const& tileset, int resolvedId) {
            if (!tileset.IsImageCollection()) {
                return atlasImages[static_cast<std::size_t>(&tileset - map.tilesets.data())];
            }
            auto const it = tileset.tileImages.find(resolvedId);
            if (it == tileset.tileImages.end()) {
                return Image{};
            }
            return resolve(it->second.imagePath);
        }, resolve);
    }
}
