#include <moth/core/log.h>
#include "moth/bridge/moth_flipbook_factory.h"
#include "moth/bridge/moth_flipbook.h"

namespace moth::bridge {
    using namespace moth::gfx;
    using namespace moth::core;
    MothFlipbookFactory::MothFlipbookFactory(SpriteSheetFactory& factoryImpl)
        : m_factoryImpl(factoryImpl) {
    }

    std::unique_ptr<moth::ui::IFlipbook> MothFlipbookFactory::GetFlipbook(std::filesystem::path const& path) {
        auto spriteSheet = m_factoryImpl.GetSpriteSheet(path);
        if (!spriteSheet) {
            return nullptr;
        }
        return std::make_unique<MothFlipbook>(std::move(spriteSheet));
    }
}
