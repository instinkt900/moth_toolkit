#pragma once

#include "moth/tilemap/tile_map.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string_view>

namespace moth::tilemap {
    /**
     * @brief Loads a Tiled JSON (.tmj) map into a @c TileMap.
     *
     * Supports orthogonal maps with embedded or external (`.tsj`) tilesets —
     * both single-atlas and image-collection tilesets — and tile layers whose
     * tile data is CSV (a JSON array), or base64 with optional zlib/gzip
     * compression. GID flip flags are unpacked per tile. Object layers are
     * parsed into @c objectLayers; image/group layers are skipped.
     *
     * Objects placed from a JSON object template (`.tj`) are resolved: the
     * instance's fields and properties override the template's, and a tile
     * template's gid is rebased onto the map's firstgid for that tileset.
     *
     * Image paths (@c Tileset::imagePath, @c TileImage::imagePath) are relative
     * to the map file, including those read from an external `.tsj` that lives
     * in another directory.
     *
     * @p basePath is the directory used to resolve a tileset's @c "source" and
     * an object's @c "template" references; pass the map file's parent directory
     * (as @c LoadTileMapFromFile does).
     *
     * Tiled does not write properties left at their class default value. Pass the
     * project's class definitions (@c LoadPropertyTypes) as @p propertyTypes to
     * fill those in on objects, maps, layers, tilesets, and tiles; values set in
     * the map (or an object's template) take precedence. Class-typed (nested)
     * properties are skipped.
     *
     * @throws std::runtime_error (or nlohmann_json exceptions) on malformed input.
     */
    TileMap LoadTileMapFromJson(nlohmann::json const& json, std::filesystem::path const& basePath = {},
                                PropertyTypes const& propertyTypes = {});

    /// @brief Loads a Tiled JSON map from a JSON string.
    TileMap LoadTileMap(std::string_view jsonText, std::filesystem::path const& basePath = {},
                        PropertyTypes const& propertyTypes = {});

    /// @brief Loads a Tiled JSON map from a file path.
    TileMap LoadTileMapFromFile(std::filesystem::path const& path, PropertyTypes const& propertyTypes = {});

    /**
     * @brief Loads the custom class definitions from a Tiled project file (`.tiled-project`).
     *
     * Only classes are kept (enum values need no definition to load). Nested
     * class-typed members are skipped.
     *
     * @throws std::runtime_error (or nlohmann_json exceptions) on malformed input.
     */
    PropertyTypes LoadPropertyTypes(std::filesystem::path const& projectPath);
}
