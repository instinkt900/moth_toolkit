#pragma once

#include "moth_ui/ui_fwd.h"

#include <memory>
#include <string>
#include <map>

namespace moth_ui {
    class LayoutCache {
    public:
        void SetLayoutRoot(char const* path);

        std::shared_ptr<Layout> GetLayout(char const* name);

        void FlushCache();

    private:
        std::string m_root;
        std::map<std::string, std::shared_ptr<Layout>> m_cache;

        std::shared_ptr<Layout> LoadLayout(char const* name);
    };
}
