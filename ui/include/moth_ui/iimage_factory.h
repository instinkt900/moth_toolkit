#pragma once

#include "moth_ui/graphics/iimage.h"

#include <filesystem>
#include <memory>

namespace moth_ui {
    class IImageFactory {
    public:
        IImageFactory(IImageFactory const&) = default;
        IImageFactory(IImageFactory&&) = default;
        IImageFactory& operator=(IImageFactory const&) = default;
        IImageFactory& operator=(IImageFactory&&) = default;
        virtual ~IImageFactory() = default;

        virtual void FlushCache() = 0;
        virtual bool LoadTexturePack(std::filesystem::path const& path) = 0;
        virtual std::unique_ptr<IImage> GetImage(std::filesystem::path const& path) = 0;
    };
}
