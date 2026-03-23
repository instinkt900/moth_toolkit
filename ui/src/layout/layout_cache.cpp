#include "common.h"
#include "moth_ui/layout/layout_cache.h"
#include "moth_ui/layout/layout.h"

namespace moth_ui {
    void LayoutCache::SetLayoutRoot(std::string_view path) {
        FlushCache();
        m_root = path;
    }

    std::shared_ptr<Layout> LayoutCache::GetLayout(std::string_view name) {
        auto const it = m_cache.find(name);
        if (std::end(m_cache) == it) {
            return LoadLayout(name);
        }
        return it->second;
    }

    void LayoutCache::FlushCache() {
        m_cache.clear();
    }

    std::shared_ptr<Layout> LayoutCache::LoadLayout(std::string_view name) {
        std::string const filename = fmt::format("{}/{}.json", m_root, name);
        std::shared_ptr<moth_ui::Layout> newLayout;
        auto const loadResult = Layout::Load(filename.c_str(), &newLayout);
        if (loadResult == moth_ui::Layout::LoadResult::Success) {
            m_cache[std::string(name)] = newLayout;
            return newLayout;
        }
        return nullptr;
    }
}
