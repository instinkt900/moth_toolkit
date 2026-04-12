#pragma once

#include "moth_ui/moth_ui_fwd.h"

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <map>

namespace moth_ui {
    /**
     * @brief In-memory cache of loaded Layout objects, keyed by name.
     *
     * Layouts are loaded on first request and reused on subsequent calls.
     * Call FlushCache() to force a reload on the next access.
     *
     * Thread safety: all public methods are thread-safe. Concurrent calls
     * to GetLayout() with the same name may each load from disk independently;
     * the first insertion wins and all callers receive the same @c shared_ptr.
     */
    class LayoutCache {
    public:
        /**
         * @brief Sets the root directory used to resolve layout names to file paths.
         * @param path Filesystem path to the root directory.
         */
        void SetLayoutRoot(std::string_view path);

        /**
         * @brief Returns the layout with the given name, loading it from disk if necessary.
         * @param name Layout name (relative to the root directory, without extension).
         * @return Shared pointer to the layout, or @c nullptr on failure.
         */
        std::shared_ptr<Layout> GetLayout(std::string_view name);

        /// @brief Removes all cached layouts so they are reloaded on next access.
        void FlushCache();

    private:
        mutable std::mutex m_mutex;
        std::string m_root;
        std::map<std::string, std::shared_ptr<Layout>, std::less<>> m_cache;

        std::shared_ptr<Layout> LoadLayout(std::string_view name);
    };
}
