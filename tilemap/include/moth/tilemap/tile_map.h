#pragma once

#include "moth/core/rect.h"
#include "moth/core/vector.h"

#include "moth/tilemap/tile.h"

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

namespace moth::tilemap {
    using moth::core::FloatRect;
    using moth::core::FloatVec2;
    using moth::core::IntRect;
    using moth::core::IntVec2;
    using moth::core::MakeRect;

    /**
     * @brief A Tiled tileset: an image atlas plus the id -> source-rect mapping.
     */
    struct Tileset {
        std::string name;
        std::string imagePath; ///< Atlas image path from the TMJ (image loading is the caller's job).
        int firstGid = 1;      ///< First global tile id owned by this tileset.
        int tileWidth = 0;
        int tileHeight = 0;
        int columns = 0;
        int tileCount = 0;
        int margin = 0;
        int spacing = 0;
        int imageWidth = 0;
        int imageHeight = 0;

        /// @brief Returns @c true if this tileset owns @p gid.
        bool ContainsGid(std::uint32_t gid) const {
            return gid >= static_cast<std::uint32_t>(firstGid)
                && gid < static_cast<std::uint32_t>(firstGid) + static_cast<std::uint32_t>(tileCount);
        }

        /// @brief Converts a global tile id to a local (0-based) id within this tileset.
        int LocalId(std::uint32_t gid) const {
            return static_cast<int>(gid) - firstGid;
        }

        /// @brief Returns the source rect of @p localId in the atlas image.
        IntRect GetTileRect(int localId) const {
            int const col = localId % columns;
            int const row = localId / columns;
            int const x = margin + col * (tileWidth + spacing);
            int const y = margin + row * (tileHeight + spacing);
            return MakeRect(x, y, tileWidth, tileHeight);
        }
    };

    /**
     * @brief A tile layer: a grid of tile references plus visibility/opacity.
     */
    struct Layer {
        std::string name;
        bool visible = true;
        float opacity = 1.0f;
        int width = 0;
        int height = 0;
        std::vector<TileId> tiles; ///< width * height, row-major (index = y * width + x).

        /// @brief Returns the tile at (@p x, @p y), or an empty tile if out of bounds.
        TileId GetTile(int x, int y) const {
            if (x < 0 || y < 0 || x >= width || y >= height) {
                return {};
            }
            return tiles[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)];
        }

        /// @brief Writes the tile at (@p x, @p y); ignores out-of-bounds coordinates.
        void SetTile(int x, int y, TileId tile) {
            if (x < 0 || y < 0 || x >= width || y >= height) {
                return;
            }
            tiles[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] = tile;
        }
    };

    /**
     * @brief A grid-based tile map: layers + tilesets, world<->tile math, and queries.
     *
     * Coordinates are map pixels with the origin at the map's top-left corner,
     * +x right and +y down (matching Tiled). The atlas image for each tileset is
     * loaded separately by the caller (see the renderer).
     */
    class TileMap {
    public:
        int width = 0;      ///< Map width in tiles.
        int height = 0;     ///< Map height in tiles.
        int tileWidth = 0;  ///< Tile width in pixels.
        int tileHeight = 0; ///< Tile height in pixels.
        std::vector<Layer> layers;
        std::vector<Tileset> tilesets;

        /// @brief Returns the map size in tiles.
        IntVec2 GetSize() const { return { width, height }; }

        /// @brief Returns the tile size in pixels.
        IntVec2 GetTileSize() const { return { tileWidth, tileHeight }; }

        /// @brief Converts a world position (map pixels) to tile coordinates.
        IntVec2 WorldToTile(FloatVec2 worldPos) const {
            return { static_cast<int>(std::floor(worldPos.x / static_cast<float>(tileWidth))),
                     static_cast<int>(std::floor(worldPos.y / static_cast<float>(tileHeight))) };
        }

        /// @brief Returns the top-left world position (map pixels) of tile (@p x, @p y).
        FloatVec2 TileToWorld(int x, int y) const {
            return { static_cast<float>(x * tileWidth), static_cast<float>(y * tileHeight) };
        }

        /// @brief Returns the number of layers.
        std::size_t GetLayerCount() const { return layers.size(); }

        /// @brief Returns layer @p index.
        Layer const& GetLayer(std::size_t index) const { return layers[index]; }

        /// @brief Returns layer @p index.
        Layer& GetLayer(std::size_t index) { return layers[index]; }

        /// @brief Returns the number of tilesets.
        std::size_t GetTilesetCount() const { return tilesets.size(); }

        /// @brief Returns tileset @p index.
        Tileset const& GetTileset(std::size_t index) const { return tilesets[index]; }

        /// @brief Returns tileset @p index.
        Tileset& GetTileset(std::size_t index) { return tilesets[index]; }

        /// @brief Returns the tileset owning @p gid, or @c nullptr.
        Tileset const* FindTileset(std::uint32_t gid) const {
            for (auto const& tileset : tilesets) {
                if (tileset.ContainsGid(gid)) {
                    return &tileset;
                }
            }
            return nullptr;
        }

        /// @brief Returns the tile in layer @p layerIndex at tile (@p x, @p y), or empty if out of bounds.
        TileId GetTile(std::size_t layerIndex, int x, int y) const {
            if (layerIndex >= layers.size()) {
                return {};
            }
            return layers[layerIndex].GetTile(x, y);
        }

        /// @brief Returns the tile in layer @p layerIndex under @p worldPos, or empty if out of bounds.
        TileId GetTileAtWorld(std::size_t layerIndex, FloatVec2 worldPos) const {
            auto const tile = WorldToTile(worldPos);
            return GetTile(layerIndex, tile.x, tile.y);
        }
    };
}
