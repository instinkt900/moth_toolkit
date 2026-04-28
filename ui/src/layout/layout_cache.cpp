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
        {
            std::lock_guard lock(m_mutex);
            auto const it = m_cache.find(name);
            if (std::end(m_cache) != it) {
                return it->second;
            }
        }

        // Cache miss: load from disk without holding the lock.
        std::shared_ptr<Layout> newLayout = LoadLayout(name);
        if (!newLayout) {
            return nullptr;
        }

        // Re-acquire to insert; a concurrent thread may have loaded and inserted
        // the same layout while we were on disk, so let the first insertion win.
        std::lock_guard lock(m_mutex);
        auto const result = m_cache.try_emplace(std::string(name), std::move(newLayout));
        return result.first->second;
    }

    void LayoutCache::FlushCache() {
        std::lock_guard lock(m_mutex);
        m_cache.clear();
    }

    std::shared_ptr<Layout> LayoutCache::LoadLayout(std::string_view name) {
        std::string const filename = fmt::format("{}/{}.json", m_root, name);
        
        auto [newLayout, loadResult] = Layout::Load(filename.c_str());
        if (loadResult == moth_ui::Layout::LoadResult::Success) {
            return newLayout;
        }
        return nullptr;
    }
}
