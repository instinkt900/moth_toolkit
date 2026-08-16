#include "moth/tilemap/tile_map_loader.h"

#include "moth/core/color.h"

#include <zlib.h>

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

        std::vector<std::uint8_t> Decompress(std::string const& compression, std::vector<std::uint8_t> const& input, std::size_t expectedSize) {
            if (compression != "zlib" && compression != "gzip") {
                throw std::runtime_error("TMJ tile data compression '" + compression + "' is not supported");
            }

            z_stream stream{};
            stream.next_in = reinterpret_cast<Bytef*>(const_cast<std::uint8_t*>(input.data()));
            stream.avail_in = static_cast<uInt>(input.size());

            // windowBits = 15 + 32 auto-detects the zlib vs gzip header.
            if (inflateInit2(&stream, 15 + 32) != Z_OK) {
                throw std::runtime_error("Failed to initialise zlib decompression");
            }

            std::vector<std::uint8_t> output(expectedSize);
            stream.next_out = output.data();
            stream.avail_out = static_cast<uInt>(expectedSize);

            int const result = inflate(&stream, Z_FINISH);
            inflateEnd(&stream);
            if (result != Z_STREAM_END) {
                throw std::runtime_error("Failed to decompress TMJ tile data");
            }
            return output;
        }

        std::vector<std::uint32_t> DecodeGids(nlohmann::json const& data, std::size_t expectedCount, std::string const& compression) {
            std::vector<std::uint32_t> gids;
            gids.reserve(expectedCount);

            if (data.is_array()) {
                for (auto const& value : data) {
                    gids.push_back(value.get<std::uint32_t>());
                }
            } else if (data.is_string()) {
                std::vector<std::uint8_t> bytes = DecodeBase64(data.get<std::string>());
                if (!compression.empty()) {
                    bytes = Decompress(compression, bytes, expectedCount * 4);
                }
                if (bytes.size() % 4 != 0) {
                    throw std::runtime_error("TMJ tile data length is not a multiple of 4");
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

        moth::core::Color ParseColor(std::string const& text) {
            if (text.size() >= 2 && text[0] == '#') {
                try {
                    std::uint32_t const argb = static_cast<std::uint32_t>(std::stoul(text.substr(1), nullptr, 16));
                    return moth::core::FromARGB(argb);
                } catch (std::exception const&) {
                    // fall through to the default
                }
            }
            return moth::core::Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        }

        Properties ParseProperties(nlohmann::json const& json) {
            Properties props;
            if (!json.is_array()) {
                return props;
            }
            for (auto const& entry : json) {
                std::string const name = entry.value("name", std::string{});
                std::string const type = entry.value("type", std::string{});
                if (name.empty()) {
                    continue;
                }
                if (type == "bool") {
                    props[name] = entry.value("value", false);
                } else if (type == "int") {
                    props[name] = entry.value("value", 0);
                } else if (type == "float") {
                    props[name] = entry.value("value", 0.0f);
                } else if (type == "color") {
                    props[name] = ParseColor(entry.value("value", std::string{}));
                } else {
                    // "string", "file", "object", "class", and unknown types -> string.
                    props[name] = entry.value("value", std::string{});
                }
            }
            return props;
        }

        nlohmann::json ReadJsonFile(std::filesystem::path const& path) {
            std::ifstream file(path, std::ios::binary);
            if (!file) {
                throw std::runtime_error("Failed to open JSON file: " + path.string());
            }
            std::ostringstream contents;
            contents << file.rdbuf();
            return nlohmann::json::parse(contents.str());
        }

        Tileset ParseTileset(nlohmann::json const& entry, int defaultTileWidth, int defaultTileHeight) {
            Tileset tileset;
            tileset.firstGid = entry.value("firstgid", 0);
            tileset.name = entry.value("name", std::string{});
            tileset.imagePath = entry.value("image", std::string{});
            tileset.tileWidth = entry.value("tilewidth", defaultTileWidth);
            tileset.tileHeight = entry.value("tileheight", defaultTileHeight);
            tileset.tileCount = entry.value("tilecount", 0);
            tileset.margin = entry.value("margin", 0);
            tileset.spacing = entry.value("spacing", 0);
            tileset.imageWidth = entry.value("imagewidth", 0);
            tileset.imageHeight = entry.value("imageheight", 0);
            tileset.columns = entry.value("columns", 0);
            if (tileset.columns <= 0 && tileset.tileWidth > 0 && tileset.imageWidth > 0) {
                tileset.columns = tileset.imageWidth / tileset.tileWidth;
            }
            if (entry.contains("properties")) {
                tileset.properties = ParseProperties(entry["properties"]);
            }
            if (entry.contains("tiles") && entry["tiles"].is_array()) {
                for (auto const& tileEntry : entry["tiles"]) {
                    int const tileId = tileEntry.value("id", 0);
                    if (tileEntry.contains("properties")) {
                        tileset.tileProperties[tileId] = ParseProperties(tileEntry["properties"]);
                    }
                    if (tileEntry.contains("animation") && tileEntry["animation"].is_array()) {
                        std::vector<AnimationFrame> frames;
                        for (auto const& frameEntry : tileEntry["animation"]) {
                            AnimationFrame frame;
                            frame.tileId = frameEntry.value("tileid", 0);
                            frame.durationMs = frameEntry.value("duration", 0);
                            frames.push_back(frame);
                        }
                        tileset.animations[tileId] = std::move(frames);
                    }
                }
            }
            return tileset;
        }
    }

    TileMap LoadTileMapFromJson(nlohmann::json const& json, std::filesystem::path const& basePath) {
        TileMap map;
        map.width = json.value("width", 0);
        map.height = json.value("height", 0);
        map.tileWidth = json.value("tilewidth", 0);
        map.tileHeight = json.value("tileheight", 0);
        map.infinite = json.value("infinite", false);

        if (map.width <= 0 || map.height <= 0 || map.tileWidth <= 0 || map.tileHeight <= 0) {
            throw std::runtime_error("TMJ map must have positive width, height, tilewidth, and tileheight");
        }

        if (json.contains("properties")) {
            map.properties = ParseProperties(json["properties"]);
        }

        if (json.contains("tilesets") && json["tilesets"].is_array()) {
            for (auto const& entry : json["tilesets"]) {
                if (entry.contains("source")) {
                    // External tileset: the .tsj holds the tileset data, the map
                    // entry holds only firstgid + source path.
                    std::filesystem::path const tsjPath = basePath / entry.value("source", std::string{});
                    Tileset tileset = ParseTileset(ReadJsonFile(tsjPath), map.tileWidth, map.tileHeight);
                    tileset.firstGid = entry.value("firstgid", 0);
                    map.tilesets.push_back(std::move(tileset));
                } else {
                    map.tilesets.push_back(ParseTileset(entry, map.tileWidth, map.tileHeight));
                }
            }
        }

        if (json.contains("layers") && json["layers"].is_array()) {
            for (auto const& entry : json["layers"]) {
                std::string const layerType = entry.value("type", std::string{});

                if (layerType == "tilelayer") {
                    std::string const compression = entry.value("compression", std::string{});

                    Layer layer;
                    layer.name = entry.value("name", std::string{});
                    layer.visible = entry.value("visible", true);
                    layer.opacity = entry.value("opacity", 1.0f);
                    layer.width = entry.value("width", map.width);
                    layer.height = entry.value("height", map.height);
                    layer.infinite = map.infinite;
                    if (entry.contains("properties")) {
                        layer.properties = ParseProperties(entry["properties"]);
                    }

                    if (layer.infinite) {
                        if (entry.contains("chunks") && entry["chunks"].is_array()) {
                            for (auto const& chunkEntry : entry["chunks"]) {
                                Chunk chunk;
                                chunk.x = chunkEntry.value("x", 0);
                                chunk.y = chunkEntry.value("y", 0);
                                auto const gids = DecodeGids(chunkEntry.at("data"),
                                                              static_cast<std::size_t>(kChunkSize) * kChunkSize,
                                                              compression);
                                for (std::size_t i = 0; i < gids.size() && i < chunk.tiles.size(); ++i) {
                                    chunk.tiles[i] = TileId::FromGid(gids[i]);
                                }
                                layer.chunks.push_back(std::move(chunk));
                            }
                        }
                    } else {
                        auto const gids = DecodeGids(entry.at("data"), static_cast<std::size_t>(layer.width) * static_cast<std::size_t>(layer.height), compression);
                        layer.tiles.reserve(gids.size());
                        for (auto const gid : gids) {
                            layer.tiles.push_back(TileId::FromGid(gid));
                        }
                    }
                    map.layers.push_back(std::move(layer));
                } else if (layerType == "objectgroup") {
                    ObjectLayer objectLayer;
                    objectLayer.name = entry.value("name", std::string{});
                    objectLayer.visible = entry.value("visible", true);
                    objectLayer.opacity = entry.value("opacity", 1.0f);
                    if (entry.contains("properties")) {
                        objectLayer.properties = ParseProperties(entry["properties"]);
                    }

                    if (entry.contains("objects") && entry["objects"].is_array()) {
                        for (auto const& object : entry["objects"]) {
                            MapObject mapObject;
                            mapObject.id = object.value("id", 0);
                            mapObject.name = object.value("name", std::string{});
                            mapObject.type = object.value("type", std::string{});
                            mapObject.position = { object.value("x", 0.0f), object.value("y", 0.0f) };
                            mapObject.size = { object.value("width", 0.0f), object.value("height", 0.0f) };
                            mapObject.rotation = object.value("rotation", 0.0f);

                            if (object.value("point", false)) {
                                mapObject.kind = ObjectKind::Point;
                            } else if (object.value("ellipse", false)) {
                                mapObject.kind = ObjectKind::Ellipse;
                            } else if (object.contains("polygon") && object["polygon"].is_array()) {
                                mapObject.kind = ObjectKind::Polygon;
                                for (auto const& point : object["polygon"]) {
                                    mapObject.points.push_back({ point.value("x", 0.0f), point.value("y", 0.0f) });
                                }
                            } else if (object.contains("polyline") && object["polyline"].is_array()) {
                                mapObject.kind = ObjectKind::Polyline;
                                for (auto const& point : object["polyline"]) {
                                    mapObject.points.push_back({ point.value("x", 0.0f), point.value("y", 0.0f) });
                                }
                            } else {
                                mapObject.kind = ObjectKind::Rectangle;
                            }

                            if (object.contains("properties")) {
                                mapObject.properties = ParseProperties(object["properties"]);
                            }

                            objectLayer.objects.push_back(std::move(mapObject));
                        }
                    }
                    map.objectLayers.push_back(std::move(objectLayer));
                }
            }
        }

        return map;
    }

    TileMap LoadTileMap(std::string_view jsonText, std::filesystem::path const& basePath) {
        return LoadTileMapFromJson(nlohmann::json::parse(jsonText), basePath);
    }

    TileMap LoadTileMapFromFile(std::filesystem::path const& path) {
        nlohmann::json const json = ReadJsonFile(path);
        return LoadTileMapFromJson(json, path.parent_path());
    }
}
