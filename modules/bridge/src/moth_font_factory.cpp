#include <spdlog/spdlog.h>
#include "moth/bridge/moth_font_factory.h"
#include "moth_graphics/graphics/font_factory.h"
#include "moth/bridge/moth_font.h"

namespace moth::bridge {
    using namespace moth::gfx::graphics;
    using namespace moth::core;
    namespace graphics = moth::gfx::graphics;
    MothFontFactory::MothFontFactory(moth::gfx::graphics::FontFactory& factoryImpl)
        : m_factoryImpl(factoryImpl) {
    }

    void MothFontFactory::ClearFonts() {
        m_factoryImpl.ClearFonts();
    }

    std::shared_ptr<moth::ui::IFont> MothFontFactory::GetFont(std::string const& name, int size) {
        auto it = m_fontPaths.find(name);
        if (it != m_fontPaths.end()) {
           auto const font = m_factoryImpl.GetFont(it->second.string(), size);
           if (!font) {
               return nullptr;
           }
           return std::make_shared<MothFont>(font);
        }
        spdlog::warn("MothFontFactory: font '{}' not registered (call AddFont first)", name);
        return nullptr;
    }
}
