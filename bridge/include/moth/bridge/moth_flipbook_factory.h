#pragma once

#include "moth_graphics/graphics/spritesheet_factory.h"

#include <moth_ui/iflipbook_factory.h>

#include <filesystem>
#include <memory>

namespace moth::bridge {
    class MothFlipbookFactory : public moth::ui::IFlipbookFactory {
    public:
        explicit MothFlipbookFactory(moth::gfx::graphics::SpriteSheetFactory& factoryImpl);
        ~MothFlipbookFactory() override = default;

        std::unique_ptr<moth::ui::IFlipbook> GetFlipbook(std::filesystem::path const& path) override;

    private:
        moth::gfx::graphics::SpriteSheetFactory& m_factoryImpl;
    };
}
