#pragma once

#include "moth_ui/iimage.h"

#include <filesystem>
#include <memory>

namespace moth_ui {
    class IImageFactory {
    public:
        virtual ~IImageFactory() = default;

        virtual void FlushCache() = 0;
        virtual bool LoadTexturePack(std::filesystem::path const& path) = 0;
        virtual std::unique_ptr<IImage> GetImage(std::filesystem::path const& path) = 0;
    };
}
