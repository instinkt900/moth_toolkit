#pragma once

#include "moth_graphics/graphics/spritesheet.h"
#include "moth/bridge/moth_image.h"

#include <moth_ui/graphics/iflipbook.h>

#include <memory>

namespace moth::bridge {
    class MothFlipbook : public moth::ui::IFlipbook {
    public:
        explicit MothFlipbook(std::shared_ptr<moth::gfx::SpriteSheet> spriteSheet);
        ~MothFlipbook() override = default;

        moth::ui::IImage const& GetImage() const override;
        int GetFrameCount() const override;
        bool GetFrameDesc(int index, moth::ui::IFlipbook::FrameDesc& outDesc) const override;
        int GetClipCount() const override;
        std::string_view GetClipName(int index) const override;
        bool GetClipDesc(std::string_view name, moth::ui::IFlipbook::ClipDesc& outDesc) const override;

    private:
        std::shared_ptr<moth::gfx::SpriteSheet> m_spriteSheet;
        MothImage m_image;
    };
}
