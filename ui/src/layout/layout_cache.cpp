#include "common.h"
#include "moth_ui/layout/layout_cache.h"
#include "moth_ui/layout/layout.h"

namespace moth_ui {
    void LayoutCache::SetLayoutRoot(char const* path) {
        FlushCache();
        m_root = path;
    }

    std::shared_ptr<Layout> LayoutCache::GetLayout(char const* name) {
        auto const it = m_cache.find(name);
        if (std::end(m_cache) == it) {
            return LoadLayout(name);
        }
        return it->second;
    }

    void LayoutCache::FlushCache() {
        m_cache.clear();
    }

    std::shared_ptr<Layout> LayoutCache::LoadLayout(char const* name) {
        std::string const filename = fmt::format("{}/{}.json", m_root, name);
        auto const layout = Layout::Load(filename.c_str());
        if (layout) {
            m_cache[name] = layout;
            return layout;
        }
        return nullptr;
    }
}
