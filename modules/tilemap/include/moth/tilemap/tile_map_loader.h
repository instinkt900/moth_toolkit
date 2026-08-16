#pragma once

#include "moth/tilemap/tile_map.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string_view>

namespace moth::tilemap {
    /**
     * @brief Loads a Tiled JSON (.tmj) map into a @c TileMap.
     *
     * Supports orthogonal maps with embedded tilesets and tile layers whose
     * tile data is CSV (a JSON array) or uncompressed base64. GID flip flags are
     * unpacked per tile. Object layers are parsed into @c objectLayers; object
     * image/group layers are skipped; compressed tile data is not yet supported.
     *
     * @p basePath is the directory used to resolve a tileset's @c "source"
     * reference to an external `.tsj` file; pass the map file's parent directory
     * (as @c LoadTileMapFromFile does).
     *
     * @throws std::runtime_error (or nlohmann_json exceptions) on malformed input.
     */
    TileMap LoadTileMapFromJson(nlohmann::json const& json, std::filesystem::path const& basePath = {});

    /// @brief Loads a Tiled JSON map from a JSON string.
    TileMap LoadTileMap(std::string_view jsonText, std::filesystem::path const& basePath = {});

    /// @brief Loads a Tiled JSON map from a file path.
    TileMap LoadTileMapFromFile(std::filesystem::path const& path);
}
