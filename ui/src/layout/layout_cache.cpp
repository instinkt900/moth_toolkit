#include "common.h"
#include "moth_ui/layout/layout_cache.h"
#include "moth_ui/layout/layout.h"

namespace moth_ui {
    void LayoutCache::SetLayoutRoot(std::string_view path) {
        std::lock_guard lock(m_mutex);
        m_cache.clear();
        m_root = path;
    }

    std::shared_ptr<Layout> LayoutCache::GetLayout(std::string_view name) {
        std::lock_guard lock(m_mutex);
        auto const it = m_cache.find(name);
        if (std::end(m_cache) == it) {
            return LoadLayout(name);
        }
        return it->second;
    }

    void LayoutCache::FlushCache() {
        std::lock_guard lock(m_mutex);
        m_cache.clear();
    }

    std::shared_ptr<Layout> LayoutCache::LoadLayout(std::string_view name) {
        std::string const filename = fmt::format("{}/{}.json", m_root, name);
        std::shared_ptr<moth_ui::Layout> newLayout;
        auto const loadResult = Layout::Load(filename.c_str(), &newLayout);
        if (loadResult == moth_ui::Layout::LoadResult::Success) {
            auto insertResult = m_cache.try_emplace(std::string(name), newLayout);
            if (insertResult.second) {
                return newLayout;
            }
        }
        return nullptr;
    }
}
