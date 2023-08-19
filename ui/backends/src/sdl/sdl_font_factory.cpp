#include "common.h"
#include "sdl/sdl_font_factory.h"
#include "sdl/sdl_font.h"

namespace backend::sdl {
    FontFactory::FontFactory(SDL_Renderer& renderer)
        : m_renderer(renderer) {
    }

    void FontFactory::AddFont(char const* name, std::filesystem::path const& path) {
        m_fontPaths[name] = path;
    }

    std::shared_ptr<moth_ui::IFont> FontFactory::GetDefaultFont(int size) {
        return GetFont(m_fontPaths.begin()->first.c_str(), size);
    }

    void FontFactory::ClearFonts() {
        m_fontPaths.clear();
    }

    std::vector<std::string> FontFactory::GetFontNameList() {
        std::vector<std::string> nameList;
        for (auto& [fontName, fontPath] : m_fontPaths) {
            nameList.push_back(fontName);
        }
        return nameList;
    }

    std::shared_ptr<moth_ui::IFont> FontFactory::GetFont(char const* name, int size) {
        assert(!m_fontPaths.empty() && "No known fonts.");
        auto const it = m_fontPaths.find(name);
        if (std::end(m_fontPaths) == it) {
            return GetDefaultFont(size);
        }
        SDL_Color defaultColor{ 0x00, 0x00, 0x00, 0xFF };
        return std::make_unique<Font>(CreateCachedFontRef(&m_renderer, it->second.string().c_str(), size, defaultColor, TTF_STYLE_NORMAL));
    }
}
