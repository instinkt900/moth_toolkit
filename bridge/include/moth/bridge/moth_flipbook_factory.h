#pragma once

#include "moth_graphics/graphics/spritesheet_factory.h"

#include <moth_ui/iflipbook_factory.h>

#include <filesystem>
#include <memory>

namespace moth::bridge {
    class MothFlipbookFactory : public moth_ui::IFlipbookFactory {
    public:
        explicit MothFlipbookFactory(moth_graphics::graphics::SpriteSheetFactory& factoryImpl);
        ~MothFlipbookFactory() override = default;

        std::unique_ptr<moth_ui::IFlipbook> GetFlipbook(std::filesystem::path const& path) override;

    private:
        moth_graphics::graphics::SpriteSheetFactory& m_factoryImpl;
    };
}
