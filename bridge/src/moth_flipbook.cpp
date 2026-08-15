#include <spdlog/spdlog.h>
#include "moth/bridge/moth_flipbook.h"

namespace moth::bridge {
    using namespace moth::gfx::graphics;
    using namespace moth::core;
    namespace graphics = moth::gfx::graphics;
    namespace {
        moth::ui::IFlipbook::LoopType ToMothLoopType(graphics::SpriteSheet::LoopType t) {
            switch (t) {
            case graphics::SpriteSheet::LoopType::Stop:  return moth::ui::IFlipbook::LoopType::Stop;
            case graphics::SpriteSheet::LoopType::Reset: return moth::ui::IFlipbook::LoopType::Reset;
            case graphics::SpriteSheet::LoopType::Loop:  return moth::ui::IFlipbook::LoopType::Loop;
            default:                                      return moth::ui::IFlipbook::LoopType::Stop;
            }
        }
    }

    MothFlipbook::MothFlipbook(std::shared_ptr<graphics::SpriteSheet> spriteSheet)
        : m_spriteSheet(spriteSheet)
        , m_image(spriteSheet->GetImage()) {
    }

    moth::ui::IImage const& MothFlipbook::GetImage() const {
        return m_image;
    }

    int MothFlipbook::GetFrameCount() const {
        return m_spriteSheet->GetFrameCount();
    }

    bool MothFlipbook::GetFrameDesc(int index, moth::ui::IFlipbook::FrameDesc& outDesc) const {
        auto entry = m_spriteSheet->GetFrameDesc(index);
        if (!entry) {
            return false;
        }
        outDesc.rect  = entry->rect;
        outDesc.pivot = entry->pivot;
        return true;
    }

    int MothFlipbook::GetClipCount() const {
        return m_spriteSheet->GetClipCount();
    }

    std::string_view MothFlipbook::GetClipName(int index) const {
        return m_spriteSheet->GetClipName(index);
    }

    bool MothFlipbook::GetClipDesc(std::string_view name, moth::ui::IFlipbook::ClipDesc& outDesc) const {
        auto internal = m_spriteSheet->GetClipDesc(name);
        if (!internal) {
            return false;
        }
        outDesc.loop = ToMothLoopType(internal->loop);
        outDesc.frames.clear();
        outDesc.frames.reserve(internal->frames.size());
        for (auto const& step : internal->frames) {
            moth::ui::IFlipbook::ClipFrame f;
            f.frameIndex = step.frameIndex;
            f.durationMs = step.durationMs;
            outDesc.frames.push_back(f);
        }
        return true;
    }
}
