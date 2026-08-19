#include "common.h"
#include "moth/anim/animator_factory.h"
#include "moth/graphics/graphics/spritesheet.h"
#include "moth/graphics/graphics/spritesheet_factory.h"

namespace moth::anim {
namespace {

std::optional<std::string> ReadFile(std::filesystem::path const& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        moth::core::log::error("AnimatorFactory: failed to open '{}'", path.string());
        return std::nullopt;
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return text;
}

// Returns false if any clip referenced by the set is missing from the sheet, or
// if a transition clip is a looping clip (which would never complete and stall
// the state machine).
bool ValidateAgainstSheet(AnimSet const& set, gfx::SpriteSheet const& sheet) {
    for (auto const& state : set.states) {
        if (!sheet.GetClipDesc(state.clip).has_value()) {
            moth::core::log::error("AnimatorFactory: state '{}' references clip '{}' not found in flipbook",
                                   state.id, state.clip);
            return false;
        }
    }
    for (auto const& transition : set.transitions) {
        if (!transition.clip.has_value()) {
            continue;
        }
        auto const clip = sheet.GetClipDesc(*transition.clip);
        if (!clip.has_value()) {
            moth::core::log::error("AnimatorFactory: transition '{}' references clip '{}' not found in flipbook",
                                   transition.id, *transition.clip);
            return false;
        }
        if (clip->loop == gfx::SpriteSheet::LoopType::Loop) {
            moth::core::log::error("AnimatorFactory: transition '{}' clip '{}' is a looping clip; transition clips must end",
                                   transition.id, *transition.clip);
            return false;
        }
    }
    return true;
}

} // namespace

    AnimatorFactory::AnimatorFactory(gfx::AssetContext& context)
        : m_context(context) {
    }

    void AnimatorFactory::FlushCache() {
        m_cache.clear();
    }

    std::shared_ptr<Animator> AnimatorFactory::CreateAnimator(std::filesystem::path const& path) {
        std::error_code ec;
        auto const absPath = std::filesystem::absolute(path, ec);
        if (ec) {
            moth::core::log::error("AnimatorFactory: failed to resolve path '{}': {}", path.string(), ec.message());
            return nullptr;
        }
        auto const key = absPath.lexically_normal().string();

        auto const cacheIt = m_cache.find(key);
        if (cacheIt != m_cache.end()) {
            return std::make_shared<Animator>(cacheIt->second.sheet, cacheIt->second.set);
        }

        auto const text = ReadFile(absPath);
        if (!text) {
            return nullptr;
        }

        auto set = ParseAnimSet(*text);
        if (!set) {
            return nullptr;
        }

        auto const rootPath = absPath.parent_path();
        auto const flipbookPath = std::filesystem::absolute(rootPath / set->flipbook, ec).lexically_normal();
        if (ec) {
            moth::core::log::error("AnimatorFactory: failed to resolve flipbook path '{}': {}",
                                   set->flipbook, ec.message());
            return nullptr;
        }

        auto sheet = m_context.GetSpriteSheetFactory().GetSpriteSheet(flipbookPath);
        if (!sheet) {
            moth::core::log::error("AnimatorFactory: failed to load flipbook '{}'", flipbookPath.string());
            return nullptr;
        }

        if (!ValidateAgainstSheet(*set, *sheet)) {
            return nullptr;
        }

        m_cache.insert({ key, Entry{ *set, sheet } });
        return std::make_shared<Animator>(std::move(sheet), std::move(*set));
    }

} // namespace moth::anim
