#pragma once

#include "moth/anim/anim_set.h"
#include "moth/anim/animator.h"
#include "moth/graphics/graphics/asset_context.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace moth::anim {
    /// @brief Loads AnimSets and their flipbooks, and creates Animators from them.
    ///
    /// Parses .anim.json descriptors (see ParseAnimSet) and loads the referenced
    /// flipbook via the supplied gfx::AssetContext's SpriteSheetFactory. The
    /// loaded AnimSet + SpriteSheet are cached by canonical path and reused;
    /// each CreateAnimator() call returns a fresh Animator with independent
    /// playback state.
    class AnimatorFactory {
    public:
        /// @param context The asset context whose SpriteSheetFactory loads the flipbook.
        explicit AnimatorFactory(gfx::AssetContext& context);
        virtual ~AnimatorFactory() = default;

        /// @brief Release all cached AnimSets and sprite sheets.
        void FlushCache();

        /// @brief Load (or reuse a cached) AnimSet and return a fresh Animator.
        /// @param path Path to the .anim.json descriptor file.
        /// @return A new Animator, or @c nullptr on failure (details logged).
        std::shared_ptr<Animator> CreateAnimator(std::filesystem::path const& path);

    private:
        struct Entry {
            AnimSet set;
            std::shared_ptr<gfx::SpriteSheet> sheet;
        };

        gfx::AssetContext& m_context;
        std::unordered_map<std::string, Entry> m_cache;
    };
}
