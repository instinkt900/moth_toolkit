#pragma once

#include "moth_ui/utils/rect.h"
#include "moth_ui/iimage.h"
#include "moth_ui/utils/color.h"
#include "moth_ui/blend_mode.h"
#include "moth_ui/text_alignment.h"

namespace moth_ui {
    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual void PushBlendMode(BlendMode mode) = 0;
        virtual void PopBlendMode() = 0;
        virtual void PushColor(Color const& color) = 0;
        virtual void PopColor() = 0;
        virtual void DrawRect(IntRect const& rect, Color const& color, BlendMode blendMode) = 0;
        virtual void DrawFilledRect(IntRect const& rect, Color const& color, BlendMode blendMode) = 0;
        virtual void RenderImage(IImage& image, IntRect const& sourceRect, IntRect const& destRect) = 0;
        virtual void RenderText(std::string const& text, IFont& font, TextHorizAlignment horizontalAlignment, TextVertAlignment verticalAlignment, IntRect const& destRect) = 0;
    };
}
