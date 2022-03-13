#pragma once

#include "moth_ui/utils/vector.h"

namespace moth_ui {
    class IImage {
    public:
        virtual ~IImage() = default;

        virtual int GetWidth() const = 0;
        virtual int GetHeight() const = 0;
        virtual IntVec2 GetDimensions() const = 0;
    };
}
