#include "common.h"
#include "moth_graphics/graphics/spritesheet_factory.h"
#include "moth_graphics/graphics/spritesheet.h"

#include <optional>

namespace moth::gfx::graphics {
namespace {

SpriteSheet::LoopType ParseLoopType(std::string const& loopStr) {
    if (loopStr == "loop") {
        return SpriteSheet::LoopType::Loop;
    }
    if (loopStr == "reset") {
        return SpriteSheet::LoopType::Reset;
    }
    return SpriteSheet::LoopType::Stop;
}

std::optional<std::vector<SpriteSheet::FrameEntry>> ParseFrames(
    nlohmann::json const& framesJson, std::string const& path) {
    std::vector<SpriteSheet::FrameEntry> frames;
    for (auto const& frameJson : framesJson) {
        if (!frameJson.contains("x") || !frameJson.contains("y") ||
            !frameJson.contains("w") || !frameJson.contains("h")) {
            moth::core::log::error("SpriteSheetFactory: '{}' frame {} missing x/y/w/h fields — aborting",
                          path, frameJson.dump());
            return std::nullopt;
        }
        if (!frameJson["x"].is_number_integer() || !frameJson["y"].is_number_integer() ||
            !frameJson["w"].is_number_integer() || !frameJson["h"].is_number_integer()) {
            moth::core::log::error("SpriteSheetFactory: '{}' frame {} x/y/w/h must be integers — aborting",
                          path, frameJson.dump());
            return std::nullopt;
        }
        int const x = frameJson["x"].get<int>();
        int const y = frameJson["y"].get<int>();
        int const w = frameJson["w"].get<int>();
        int const h = frameJson["h"].get<int>();
        if (w <= 0 || h <= 0) {
            moth::core::log::error("SpriteSheetFactory: '{}' frame {} has non-positive w/h ({},{}) — aborting",
                          path, frameJson.dump(), w, h);
            return std::nullopt;
        }
        SpriteSheet::FrameEntry entry;
        entry.rect = MakeRect(x, y, w, h);
        entry.pivot.x = 0;
        if (frameJson.contains("pivot_x")) {
            if (!frameJson["pivot_x"].is_number_integer()) {
                moth::core::log::error("SpriteSheetFactory: '{}' pivot_x must be an integer — aborting",
                              path);
                return std::nullopt;
            }
            entry.pivot.x = frameJson["pivot_x"].get<int>();
        }
        entry.pivot.y = 0;
        if (frameJson.contains("pivot_y")) {
            if (!frameJson["pivot_y"].is_number_integer()) {
                moth::core::log::error("SpriteSheetFactory: '{}' pivot_y must be an integer — aborting",
                              path);
                return std::nullopt;
            }
            entry.pivot.y = frameJson["pivot_y"].get<int>();
        }
        frames.push_back(entry);
    }
    return frames;
}

std::optional<SpriteSheet::ClipEntry> ParseClipEntry(
    nlohmann::json const& clipJson, std::string const& path, int totalFrames) {
    if (!clipJson.contains("name") || !clipJson["name"].is_string()) {
        moth::core::log::warn("SpriteSheetFactory: '{}' skipping clip with missing or non-string 'name'",
                     path);
        return std::nullopt;
    }
    if (!clipJson.contains("frames") || !clipJson["frames"].is_array()) {
        moth::core::log::warn("SpriteSheetFactory: '{}' skipping clip '{}': missing 'frames' array",
                     path, clipJson["name"].get<std::string>());
        return std::nullopt;
    }

    SpriteSheet::ClipEntry entry;
    entry.name = clipJson["name"].get<std::string>();

    bool valid = true;
    for (auto const& stepJson : clipJson["frames"]) {
        if (!stepJson.contains("frame") || !stepJson.contains("duration_ms")) {
            moth::core::log::warn("SpriteSheetFactory: '{}' clip '{}': step missing frame/duration_ms",
                         path, entry.name);
            valid = false;
            break;
        }
        if (!stepJson["frame"].is_number_integer() || !stepJson["duration_ms"].is_number_integer()) {
            moth::core::log::warn("SpriteSheetFactory: '{}' clip '{}': frame/duration_ms must be integers",
                         path, entry.name);
            valid = false;
            break;
        }
        SpriteSheet::ClipFrame step;
        step.frameIndex = stepJson["frame"].get<int>();
        step.durationMs = stepJson["duration_ms"].get<int>();
        if (step.frameIndex < 0 || step.frameIndex >= totalFrames) {
            moth::core::log::warn("SpriteSheetFactory: '{}' clip '{}': frame index {} out of range [0, {})",
                         path, entry.name, step.frameIndex, totalFrames);
            valid = false;
            break;
        }
        if (step.durationMs <= 0) {
            moth::core::log::warn("SpriteSheetFactory: '{}' clip '{}': duration_ms must be > 0 (got {})",
                         path, entry.name, step.durationMs);
            valid = false;
            break;
        }
        entry.desc.frames.push_back(step);
    }

    if (!valid || entry.desc.frames.empty()) {
        moth::core::log::warn("SpriteSheetFactory: '{}' skipping empty or invalid clip '{}'",
                     path, entry.name);
        return std::nullopt;
    }

    std::string loopStr = "stop";
    if (clipJson.contains("loop") && clipJson["loop"].is_string()) {
        loopStr = clipJson["loop"].get<std::string>();
    }
    entry.desc.loop = ParseLoopType(loopStr);

    return entry;
}

std::vector<SpriteSheet::ClipEntry> ParseClips(
    nlohmann::json const& clipsJson, std::string const& path, int totalFrames) {
    std::vector<SpriteSheet::ClipEntry> clips;
    for (auto const& clipJson : clipsJson) {
        auto entry = ParseClipEntry(clipJson, path, totalFrames);
        if (entry) {
            clips.push_back(std::move(*entry));
        }
    }
    return clips;
}

std::shared_ptr<SpriteSheet> BuildSpriteSheet(
    nlohmann::json const& json, std::shared_ptr<ITexture> texture, std::string const& label) {
    if (!texture) {
        moth::core::log::error("SpriteSheetFactory: '{}' has no atlas texture", label);
        return nullptr;
    }
    if (!json.contains("frames") || !json["frames"].is_array()) {
        moth::core::log::error("SpriteSheetFactory: '{}' missing 'frames' array", label);
        return nullptr;
    }

    Image image(texture);

    auto frames = ParseFrames(json["frames"], label);
    if (!frames) {
        return nullptr;
    }
    if (frames->empty()) {
        moth::core::log::error("SpriteSheetFactory: '{}' frames array is empty", label);
        return nullptr;
    }

    int const totalFrames = static_cast<int>(frames->size());

    std::vector<SpriteSheet::ClipEntry> clips;
    if (json.contains("clips")) {
        if (!json["clips"].is_array()) {
            moth::core::log::error("SpriteSheetFactory: '{}' 'clips' field must be an array (got {})",
                          label, json["clips"].type_name());
            return nullptr;
        }
        clips = ParseClips(json["clips"], label, totalFrames);
    }

    return std::make_shared<SpriteSheet>(std::move(image), std::move(*frames), std::move(clips));
}

} // namespace

    SpriteSheetFactory::SpriteSheetFactory(AssetContext& context)
        : m_context(context) {
    }

    void SpriteSheetFactory::FlushCache() {
        m_cache.clear();
    }

    std::shared_ptr<SpriteSheet> SpriteSheetFactory::GetSpriteSheet(std::filesystem::path const& path) {
        std::error_code ec;
        auto const absPath = std::filesystem::absolute(path, ec);
        if (ec) {
            moth::core::log::error("SpriteSheetFactory: failed to resolve path '{}': {}", path.string(), ec.message());
            return nullptr;
        }
        auto const key = absPath.lexically_normal().string();

        auto const cacheIt = m_cache.find(key);
        if (cacheIt != std::end(m_cache)) {
            return cacheIt->second;
        }

        bool const fileExists = std::filesystem::exists(absPath, ec);
        if (ec) {
            moth::core::log::error("SpriteSheetFactory: failed to check existence of '{}': {}", path.string(), ec.message());
            return nullptr;
        }
        if (!fileExists) {
            moth::core::log::error("SpriteSheetFactory: '{}' does not exist", path.string());
            return nullptr;
        }

        std::ifstream ifile(absPath);
        if (!ifile.is_open()) {
            moth::core::log::error("SpriteSheetFactory: failed to open '{}'", absPath.string());
            return nullptr;
        }

        nlohmann::json json;
        try {
            ifile >> json;
        } catch (std::exception const& e) {
            moth::core::log::error("SpriteSheetFactory: failed to parse '{}': {}", path.string(), e.what());
            return nullptr;
        }

        if (!json.contains("image") || !json["image"].is_string()) {
            moth::core::log::error("SpriteSheetFactory: '{}' missing 'image' string field", path.string());
            return nullptr;
        }

        ec = {};
        auto const rootPath = path.parent_path();
        auto const imageAbsPath = std::filesystem::absolute(
            rootPath / json["image"].get<std::string>(), ec).lexically_normal();
        if (ec) {
            moth::core::log::error("SpriteSheetFactory: '{}' failed to resolve image path: {}", path.string(), ec.message());
            return nullptr;
        }

        std::shared_ptr<ITexture> texture(m_context.TextureFromFile(imageAbsPath));
        if (!texture) {
            moth::core::log::error("SpriteSheetFactory: '{}' failed to load image '{}'",
                          path.string(), imageAbsPath.string());
            return nullptr;
        }

        auto sheet = BuildSpriteSheet(json, std::move(texture), path.string());
        if (sheet) {
            m_cache.insert({ key, sheet });
        }
        return sheet;
    }

    std::shared_ptr<SpriteSheet> SpriteSheetFactory::GetSpriteSheetFromMemory(
        std::vector<std::uint8_t> const& descriptorBytes,
        std::shared_ptr<ITexture> imageTexture) {
        nlohmann::json json;
        try {
            json = nlohmann::json::parse(descriptorBytes.begin(), descriptorBytes.end());
        } catch (std::exception const& e) {
            moth::core::log::error("SpriteSheetFactory: failed to parse in-memory descriptor: {}", e.what());
            return nullptr;
        }

        return BuildSpriteSheet(json, std::move(imageTexture), "<memory>");
    }
}
