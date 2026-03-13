#pragma once

#include "moth_ui/ui_fwd.h"

#include <memory>
#include <string>
#include <map>

namespace moth_ui {
    /**
     * @brief In-memory cache of loaded Layout objects, keyed by name.
     *
     * Layouts are loaded on first request and reused on subsequent calls.
     * Call FlushCache() to force a reload on the next access.
     */
    class LayoutCache {
    public:
        /**
         * @brief Sets the root directory used to resolve layout names to file paths.
         * @param path Filesystem path to the root directory.
         */
        void SetLayoutRoot(char const* path);

        /**
         * @brief Returns the layout with the given name, loading it from disk if necessary.
         * @param name Layout name (relative to the root directory, without extension).
         * @return Shared pointer to the layout, or @c nullptr on failure.
         */
        std::shared_ptr<Layout> GetLayout(char const* name);

        /// @brief Removes all cached layouts so they are reloaded on next access.
        void FlushCache();

    private:
        std::string m_root;
        std::map<std::string, std::shared_ptr<Layout>> m_cache;

        std::shared_ptr<Layout> LoadLayout(char const* name);
    };
}
