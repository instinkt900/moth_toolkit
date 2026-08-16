#include "moth/tilemap/tile_map_loader.h"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace moth::tilemap {
    namespace {
        int Base64Value(char c) {
            if (c >= 'A' && c <= 'Z') {
                return c - 'A';
            }
            if (c >= 'a' && c <= 'z') {
                return c - 'a' + 26;
            }
            if (c >= '0' && c <= '9') {
                return c - '0' + 52;
            }
            if (c == '+') {
                return 62;
            }
            if (c == '/') {
                return 63;
            }
            return -1;
        }

        std::vector<std::uint8_t> DecodeBase64(std::string_view input) {
            std::vector<std::uint8_t> out;
            out.reserve((input.size() / 4) * 3);

            int accumulator = 0;
            int bits = -8;
            for (char c : input) {
                if (c == '=' || std::isspace(static_cast<unsigned char>(c))) {
                    continue;
                }
                int const value = Base64Value(c);
                if (value < 0) {
                    throw std::runtime_error("Invalid base64 character in TMJ tile data");
                }
                accumulator = (accumulator << 6) | value;
                bits += 6;
                if (bits >= 0) {
                    out.push_back(static_cast<std::uint8_t>((accumulator >> bits) & 0xFF));
                    bits -= 8;
                }
            }
            return out;
        }

        std::vector<std::uint32_t> DecodeGids(nlohmann::json const& data, std::size_t expectedCount) {
            std::vector<std::uint32_t> gids;
            gids.reserve(expectedCount);

            if (data.is_array()) {
                for (auto const& value : data) {
                    gids.push_back(value.get<std::uint32_t>());
                }
            } else if (data.is_string()) {
                auto const bytes = DecodeBase64(data.get<std::string>());
                if (bytes.size() % 4 != 0) {
                    throw std::runtime_error("TMJ base64 tile data length is not a multiple of 4");
                }
                for (std::size_t i = 0; i < bytes.size(); i += 4) {
                    std::uint32_t gid = 0;
                    gid |= static_cast<std::uint32_t>(bytes[i + 0]);
                    gid |= static_cast<std::uint32_t>(bytes[i + 1]) << 8;
                    gid |= static_cast<std::uint32_t>(bytes[i + 2]) << 16;
                    gid |= static_cast<std::uint32_t>(bytes[i + 3]) << 24;
                    gids.push_back(gid);
                }
            } else {
                throw std::runtime_error("TMJ layer 'data' must be a CSV array or a base64 string");
            }
            return gids;
        }
    }

    TileMap LoadTileMapFromJson(nlohmann::json const& json) {
        TileMap map;
        map.width = json.value("width", 0);
        map.height = json.value("height", 0);
        map.tileWidth = json.value("tilewidth", 0);
        map.tileHeight = json.value("tileheight", 0);

        if (map.width <= 0 || map.height <= 0 || map.tileWidth <= 0 || map.tileHeight <= 0) {
            throw std::runtime_error("TMJ map must have positive width, height, tilewidth, and tileheight");
        }

        if (json.contains("tilesets") && json["tilesets"].is_array()) {
            for (auto const& entry : json["tilesets"]) {
                Tileset tileset;
                tileset.firstGid = entry.value("firstgid", 0);
                tileset.name = entry.value("name", std::string{});
                tileset.imagePath = entry.value("image", std::string{});
                tileset.tileWidth = entry.value("tilewidth", map.tileWidth);
                tileset.tileHeight = entry.value("tileheight", map.tileHeight);
                tileset.tileCount = entry.value("tilecount", 0);
                tileset.margin = entry.value("margin", 0);
                tileset.spacing = entry.value("spacing", 0);
                tileset.imageWidth = entry.value("imagewidth", 0);
                tileset.imageHeight = entry.value("imageheight", 0);
                tileset.columns = entry.value("columns", 0);
                if (tileset.columns <= 0 && tileset.tileWidth > 0 && tileset.imageWidth > 0) {
                    tileset.columns = tileset.imageWidth / tileset.tileWidth;
                }
                map.tilesets.push_back(std::move(tileset));
            }
        }

        if (json.contains("layers") && json["layers"].is_array()) {
            for (auto const& entry : json["layers"]) {
                if (entry.value("type", std::string{}) != "tilelayer") {
                    continue;
                }

                std::string const compression = entry.value("compression", std::string{});
                if (!compression.empty()) {
                    throw std::runtime_error("TMJ compressed tile data is not supported");
                }

                Layer layer;
                layer.name = entry.value("name", std::string{});
                layer.visible = entry.value("visible", true);
                layer.opacity = entry.value("opacity", 1.0f);
                layer.width = entry.value("width", map.width);
                layer.height = entry.value("height", map.height);

                auto const gids = DecodeGids(entry.at("data"), static_cast<std::size_t>(layer.width) * static_cast<std::size_t>(layer.height));
                layer.tiles.reserve(gids.size());
                for (auto const gid : gids) {
                    layer.tiles.push_back(TileId::FromGid(gid));
                }
                map.layers.push_back(std::move(layer));
            }
        }

        return map;
    }

    TileMap LoadTileMap(std::string_view jsonText) {
        return LoadTileMapFromJson(nlohmann::json::parse(jsonText));
    }

    TileMap LoadTileMapFromFile(std::filesystem::path const& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open TMJ file: " + path.string());
        }
        std::ostringstream contents;
        contents << file.rdbuf();
        return LoadTileMap(std::string_view(contents.str()));
    }
}
