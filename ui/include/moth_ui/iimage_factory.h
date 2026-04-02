#pragma once

#include "moth_ui/graphics/iimage.h"

#include <filesystem>
#include <memory>

namespace moth_ui {
    /**
     * @brief Abstract factory interface for loading and caching images.
     *
     * Consumers of the UI library must provide a concrete implementation
     * that loads images from the filesystem using the backend renderer.
     */
    class IImageFactory {
    public:
        IImageFactory() = default;
        IImageFactory(IImageFactory const&) = default;
        IImageFactory(IImageFactory&&) = default;
        IImageFactory& operator=(IImageFactory const&) = default;
        IImageFactory& operator=(IImageFactory&&) = default;
        virtual ~IImageFactory() = default;

        /// @brief Discards all cached images, forcing subsequent loads from disk.
        virtual void FlushCache() = 0;

        /**
         * @brief Returns an image loaded from the given path.
         * @param path Path to the image file.
         * @return Newly created image, or @c nullptr on failure.
         */
        virtual std::unique_ptr<IImage> GetImage(std::filesystem::path const& path) = 0;
    };
}
