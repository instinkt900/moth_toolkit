#pragma once

#include "iimage.h"

namespace moth_ui {
    class IImageFactory {
    public:
        virtual ~IImageFactory() = default;

        virtual std::unique_ptr<IImage> GetImage(char const* path) = 0;
    };
}
