#pragma once

#include "iimage.h"

namespace moth_ui {
    class IImageFactory {
    public:
        virtual ~IImageFactory() = default;

        virtual std::unique_ptr<IImage> GetImage(std::filesystem::path const& path) = 0;
    };
}
