#include "common.h"
#include "vulkan/vulkan_font_factory.h"
#include "vulkan/vulkan_font.h"

namespace backend::vulkan {
    FontFactory::FontFactory(Context& context, Graphics& graphics)
        : m_fontCache(context, graphics) {
    }

    void FontFactory::AddFont(char const* name, std::filesystem::path const& path) {
        m_fontPaths[name] = path;
    }

    void FontFactory::ClearFonts() {
        m_fontPaths.clear();
        m_fontCache.Clear();
    }

    std::shared_ptr<moth_ui::IFont> FontFactory::GetDefaultFont(int size) {
        return GetFont(m_fontPaths.begin()->first.c_str(), size);
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
        return m_fontCache.GetFont(it->second.string().c_str(), size);
    }
}
