#pragma once

#include "moth/core/rect.h"
#include "moth/core/vector.h"

#include "moth/tilemap/properties.h"
#include "moth/tilemap/tile.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace moth::tilemap {
    using moth::core::FloatRect;
    using moth::core::FloatVec2;
    using moth::core::IntRect;
    using moth::core::IntVec2;
    using moth::core::MakeRect;

    /// @brief The fixed chunk size (in tiles) of Tiled's infinite maps.
    constexpr int kChunkSize = 16;

    /// @brief A single frame of an animated tile.
    struct AnimationFrame {
        int tileId = 0;     ///< Local tile id to display for this frame.
        int durationMs = 100; ///< How long this frame is displayed, in milliseconds.
    };

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

        Properties properties;                    ///< Tileset-level custom properties.
        std::map<int, Properties> tileProperties; ///< Per-tile properties, keyed by local tile id.
        std::map<int, std::vector<AnimationFrame>> animations; ///< Animated tiles, keyed by local tile id.

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
     * @brief Resolves the local tile id to display for @p localId at @p timeMs.
     *
     * If @p localId has an animation, the frame at @p timeMs (mod the total
     * animation length) is returned; otherwise @p localId is returned unchanged.
     * All instances of an animated tile share one phase (Tiled semantics).
     */
    inline int ResolveTileId(Tileset const& tileset, int localId, std::uint32_t timeMs) {
        auto const it = tileset.animations.find(localId);
        if (it == tileset.animations.end() || it->second.empty()) {
            return localId;
        }
        auto const& frames = it->second;

        std::uint64_t total = 0;
        for (auto const& frame : frames) {
            total += static_cast<std::uint64_t>(frame.durationMs);
        }
        if (total == 0) {
            return frames.front().tileId;
        }

        std::uint64_t const t = static_cast<std::uint64_t>(timeMs) % total;
        std::uint64_t accumulated = 0;
        for (auto const& frame : frames) {
            accumulated += static_cast<std::uint64_t>(frame.durationMs);
            if (t < accumulated) {
                return frame.tileId;
            }
        }
        return frames.back().tileId;
    }

    /**
     * @brief A 16x16 chunk of tiles, positioned in chunk coordinates (not tile
     * coordinates). Used by Tiled's infinite maps.
     */
    struct Chunk {
        int x = 0; ///< Chunk coordinate (chunk index) along the horizontal axis.
        int y = 0; ///< Chunk coordinate (chunk index) along the vertical axis.
        std::array<TileId, kChunkSize * kChunkSize> tiles{}; ///< Row-major 16x16 tile grid.
    };

    /// @brief Floors @p a / @p b (both the dividend's and divisor's sign handled correctly).
    inline int FloorDiv(int a, int b) {
        int const q = a / b;
        int const r = a % b;
        return (r != 0 && ((r < 0) != (b < 0))) ? q - 1 : q;
    }

    /**
     * @brief A tile layer: a grid of tile references plus visibility/opacity.
     *
     * Finite layers use the flat @c tiles grid; infinite layers (Tiled
     * `"infinite": true`) store their data as @c chunks instead and have no
     * meaningful @c width/@c height.
     */
    struct Layer {
        std::string name;
        bool visible = true;
        float opacity = 1.0f;
        int width = 0;
        int height = 0;
        bool infinite = false;
        std::vector<TileId> tiles; ///< width * height, row-major (index = y * width + x); finite layers only.
        std::vector<Chunk> chunks; ///< 16x16 chunks; infinite layers only.

        Properties properties; ///< Layer-level custom properties.

        /// @brief Returns the tile at (@p x, @p y), or an empty tile if out of bounds/absent.
        TileId GetTile(int x, int y) const {
            if (infinite) {
                int const cx = FloorDiv(x, kChunkSize);
                int const cy = FloorDiv(y, kChunkSize);
                for (auto const& chunk : chunks) {
                    if (chunk.x == cx && chunk.y == cy) {
                        int const lx = x - cx * kChunkSize;
                        int const ly = y - cy * kChunkSize;
                        return chunk.tiles[static_cast<std::size_t>(ly) * kChunkSize + static_cast<std::size_t>(lx)];
                    }
                }
                return {};
            }
            if (x < 0 || y < 0 || x >= width || y >= height) {
                return {};
            }
            return tiles[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)];
        }

        /// @brief Writes the tile at (@p x, @p y); ignores out-of-bounds coordinates (and absent chunks).
        void SetTile(int x, int y, TileId tile) {
            if (infinite) {
                int const cx = FloorDiv(x, kChunkSize);
                int const cy = FloorDiv(y, kChunkSize);
                for (auto& chunk : chunks) {
                    if (chunk.x == cx && chunk.y == cy) {
                        int const lx = x - cx * kChunkSize;
                        int const ly = y - cy * kChunkSize;
                        chunk.tiles[static_cast<std::size_t>(ly) * kChunkSize + static_cast<std::size_t>(lx)] = tile;
                        return;
                    }
                }
                return;
            }
            if (x < 0 || y < 0 || x >= width || y >= height) {
                return;
            }
            tiles[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] = tile;
        }
    };

    /**
     * @brief The shape of a Tiled map object.
     */
    enum class ObjectKind {
        Rectangle, ///< An axis-aligned rectangle (x/y/width/height).
        Ellipse,   ///< An ellipse inscribed in the rectangle (width/height).
        Polygon,   ///< A filled polygon (points relative to the object origin).
        Polyline,  ///< An open polyline (points relative to the object origin).
        Point,     ///< A single point (no size).
    };

    /**
     * @brief A single Tiled object: a named, positioned shape.
     *
     * Position/size are in map pixels with the origin at the map's top-left
     * corner; @c rotation is in degrees (clockwise). Polygon/polyline points
     * are relative to the object's position.
     */
    struct MapObject {
        int id = 0;
        std::string name;
        std::string type;
        ObjectKind kind = ObjectKind::Rectangle;
        FloatVec2 position = { 0.0f, 0.0f };
        FloatVec2 size = { 0.0f, 0.0f };
        float rotation = 0.0f;
        std::vector<FloatVec2> points; ///< Polygon/polyline vertices (relative to position).
        Properties properties;         ///< Object-level custom properties.
    };

    /**
     * @brief An object layer: a named collection of map objects (colliders,
     * spawn points, triggers). Not rendered by @c DrawTileMap — game code
     * iterates the objects (e.g. to build physics bodies).
     */
    struct ObjectLayer {
        std::string name;
        bool visible = true;
        float opacity = 1.0f;
        std::vector<MapObject> objects;
        Properties properties; ///< Object-layer-level custom properties.
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
        bool infinite = false; ///< @c true for Tiled infinite maps (chunked tile layers).
        std::vector<Layer> layers;
        std::vector<Tileset> tilesets;
        std::vector<ObjectLayer> objectLayers;

        Properties properties; ///< Map-level custom properties.

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

        /// @brief Returns the number of object layers.
        std::size_t GetObjectLayerCount() const { return objectLayers.size(); }

        /// @brief Returns object layer @p index.
        ObjectLayer const& GetObjectLayer(std::size_t index) const { return objectLayers[index]; }

        /// @brief Returns object layer @p index.
        ObjectLayer& GetObjectLayer(std::size_t index) { return objectLayers[index]; }

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
