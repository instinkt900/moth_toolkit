#include "moth/tilemap/tile_map_collision.h"

namespace moth::tilemap {
    namespace {
        void AppendTileCollisions(TileMap const& map, TileId const& tile, FloatVec2 const& world, std::vector<MapObject>& out) {
            if (tile.IsEmpty()) {
                return;
            }
            Tileset const* tileset = map.FindTileset(tile.id);
            if (tileset == nullptr) {
                return;
            }
            auto const it = tileset->tileCollisions.find(tileset->LocalId(tile.id));
            if (it == tileset->tileCollisions.end()) {
                return;
            }
            for (auto shape : it->second) {
                shape.position += world;
                out.push_back(std::move(shape));
            }
        }
    }

    std::vector<MapObject> CollectLayerCollisions(TileMap const& map, std::size_t layerIndex) {
        std::vector<MapObject> result;
        if (layerIndex >= map.layers.size()) {
            return result;
        }
        Layer const& layer = map.layers[layerIndex];

        if (layer.infinite) {
            for (auto const& chunk : layer.chunks) {
                int const baseTx = chunk.x * kChunkSize;
                int const baseTy = chunk.y * kChunkSize;
                for (int ly = 0; ly < kChunkSize; ++ly) {
                    for (int lx = 0; lx < kChunkSize; ++lx) {
                        AppendTileCollisions(map, chunk.tiles[static_cast<std::size_t>(ly * kChunkSize + lx)],
                                             map.TileToWorld(baseTx + lx, baseTy + ly), result);
                    }
                }
            }
        } else {
            for (int ty = 0; ty < layer.height; ++ty) {
                for (int tx = 0; tx < layer.width; ++tx) {
                    AppendTileCollisions(map, layer.GetTile(tx, ty), map.TileToWorld(tx, ty), result);
                }
            }
        }
        return result;
    }
}
