#pragma once

#include "moth_graphics/graphics/asset_context.h"
#include "moth_graphics/graphics/spritesheet.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace moth::gfx {
    /// @brief Cached sprite sheet loader.
    ///
    /// Parses .flipbook.json descriptors produced by moth_packer and loads the
    /// referenced sprite sheet image via the supplied AssetContext. Loaded sprite
    /// sheets are cached by canonical path and reused on subsequent requests.
    class SpriteSheetFactory {
    public:
        /// @param context The asset context used to load the sprite sheet image.
        explicit SpriteSheetFactory(AssetContext& context);
        virtual ~SpriteSheetFactory() = default;

        /// @brief Release all cached sprite sheets.
        void FlushCache();

        /// @brief Load or retrieve a cached sprite sheet by descriptor path.
        /// @param path Path to the .flipbook.json descriptor file.
        /// @return Loaded sprite sheet, or @c nullptr on failure.
        std::shared_ptr<SpriteSheet> GetSpriteSheet(std::filesystem::path const& path);

        /// @brief Build a sprite sheet from in-memory descriptor bytes and atlas texture.
        ///
        /// The descriptor is the same .flipbook.json format produced by moth_packer;
        /// its @c "image" field is ignored (the atlas is supplied directly). Not cached.
        /// @param descriptorBytes The .flipbook.json descriptor, as bytes.
        /// @param imageTexture The atlas texture the descriptor's frames refer to.
        /// @return Loaded sprite sheet, or @c nullptr on failure.
        static std::shared_ptr<SpriteSheet> GetSpriteSheetFromMemory(
            std::vector<std::uint8_t> const& descriptorBytes,
            std::shared_ptr<ITexture> imageTexture);

    private:
        AssetContext& m_context;
        std::unordered_map<std::string, std::shared_ptr<SpriteSheet>> m_cache;
    };
}
