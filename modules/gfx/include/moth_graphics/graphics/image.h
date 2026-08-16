#pragma once

#include "moth_graphics/graphics/itexture.h"
#include "moth_graphics/utils/rect.h"
#include "moth_graphics/utils/vector.h"

#include <memory>

namespace moth::gfx::graphics {
    /// @brief A handle to a 2D image: a texture plus an optional sub-region rect.
    ///
    /// Images are cheap value types — copying one shares the underlying GPU
    /// texture via @c std::shared_ptr. Construct directly from a texture (with
    /// or without a sub-region), or obtain one from @c TextureFactory::GetTexture
    /// combined with @c GetTextureRect. Construct directly at the call site:
    /// @code
    ///   auto tex = factory.GetTexture(path);
    ///   if (tex) { Image image{ tex, factory.GetTextureRect(path) }; }
    /// @endcode
    ///
    /// A default-constructed @c Image is empty (@c operator @c bool returns
    /// @c false). Factories return an empty @c Image to indicate failure.
    class Image {
    public:
        /// @brief Construct an empty image (no texture).
        Image() = default;

        /// @brief Construct an image covering an entire texture.
        /// @param texture Backing texture. Must not be null for the image to be valid.
        explicit Image(std::shared_ptr<ITexture> texture);

        /// @brief Construct an image covering a sub-region of a texture.
        /// @param texture Backing texture. Must not be null for the image to be valid.
        /// @param sourceRect Sub-region of @p texture to display, in pixels.
        Image(std::shared_ptr<ITexture> texture, IntRect const& sourceRect);

        /// @brief Returns the width of the source rectangle in pixels.
        int GetWidth() const { return m_sourceRect.bottomRight.x - m_sourceRect.topLeft.x; }

        /// @brief Returns the height of the source rectangle in pixels.
        int GetHeight() const { return m_sourceRect.bottomRight.y - m_sourceRect.topLeft.y; }

        /// @brief Returns the underlying GPU texture (may be null for an empty image).
        std::shared_ptr<ITexture> const& GetTexture() const { return m_texture; }

        /// @brief Returns the source rectangle within the texture.
        IntRect const& GetSourceRect() const { return m_sourceRect; }

        /// @brief Returns @c true if this image has a backing texture.
        explicit operator bool() const { return m_texture != nullptr; }

    private:
        std::shared_ptr<ITexture> m_texture;
        IntRect m_sourceRect;
    };
}
