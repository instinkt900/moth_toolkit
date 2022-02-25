#pragma once

#include "moth_ui/utils/rect.h"
#include "moth_ui/iimage.h"

namespace ui {
    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual void SetRenderColor(uint32_t argb) = 0;
        virtual void RenderRect(IntRect const& rect) = 0;
        virtual void RenderFilledRect(IntRect const& rect) = 0;
        virtual void RenderImage(IImage& image, IntRect const& destRect) = 0;
    };
}
