#pragma once

#include "moth/tilemap/tile_map.h"

#include <cstddef>
#include <vector>

namespace moth::tilemap {
    /**
     * @brief Collects the collision shapes of every tile in layer @p layerIndex,
     * positioned in world (map pixel) space.
     *
     * Reads @c Tileset::tileCollisions (the Tiled collision editor shapes). Each
     * shape's @c position is offset by its tile's world position; polygon/polyline
     * points stay relative to the shape origin. Empty tiles and tiles without
     * collision shapes contribute nothing. Flip flags are ignored (Tiled does not
     * flip collision shapes either).
     *
     * Works for finite and infinite (chunked) layers. Returns an empty vector for
     * an out-of-range layer index.
     */
    std::vector<MapObject> CollectLayerCollisions(TileMap const& map, std::size_t layerIndex);
}
