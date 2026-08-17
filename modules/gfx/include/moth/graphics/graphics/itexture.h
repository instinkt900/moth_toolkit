#pragma once

#include "moth/graphics/graphics/texture_filter.h"
#include "moth/graphics/graphics/texture_address_mode.h"
#include "moth/graphics/utils/rect.h"
#include "moth/graphics/utils/vector.h"

#include <cstdint>

namespace moth::gfx {
    /// @brief Abstract GPU texture resource.
    ///
    /// Owns raw pixel data on the GPU. Wrap in an @c Image to use with draw
    /// calls, or obtain one via @c AssetContext::TextureFromFile().
    class ITexture {
    public:
        virtual ~ITexture() = default;

        /// @brief Returns the width of the texture in pixels.
        virtual int GetWidth() const = 0;

        /// @brief Returns the height of the texture in pixels.
        virtual int GetHeight() const = 0;

        /// @brief Set the minification and magnification filters.
        /// @param minFilter Filter applied when the texture is scaled down.
        /// @param magFilter Filter applied when the texture is scaled up.
        virtual void SetFilter(TextureFilter minFilter, TextureFilter magFilter) = 0;

        /// @brief Set the UV address (wrap) mode for each axis.
        /// @param u Address mode along the horizontal axis.
        /// @param v Address mode along the vertical axis.
        virtual void SetAddressMode(TextureAddressMode u, TextureAddressMode v) = 0;

        /// @brief Replace a sub-region of the texture with new RGBA pixel data.
        ///
        /// Only supported on textures created via
        /// @c AssetContext::TextureFromPixels (i.e. CPU-writable textures).
        /// @c pixels must point to a tightly packed RGBA buffer of size
        /// @c destRect.w() * destRect.h() * 4 bytes, with row pitch equal to
        /// @c destRect.w() * 4. The destination rect must lie fully within the
        /// texture's bounds.
        /// @param destRect Region of the texture to overwrite.
        /// @param pixels   Source pixel data, RGBA8 tightly packed.
        virtual void UpdatePixels(IntRect const& destRect, uint8_t const* pixels) = 0;
    };
}
