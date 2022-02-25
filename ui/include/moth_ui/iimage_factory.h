#pragma once

#include "iimage.h"

namespace ui {
    class IImageFactory {
    public:
        virtual ~IImageFactory() = default;

        virtual std::unique_ptr<IImage> GetImage(char const* path) = 0;
    };
}
