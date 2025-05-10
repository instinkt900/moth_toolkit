#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/graphics/blend_mode.h"
#include "moth_ui/graphics/image_scale_type.h"
#include "moth_ui/graphics/text_alignment.h"
#include "moth_ui/utils/color.h"
#include "moth_ui/utils/rect.h"

#include <string>

namespace moth_ui {
    class IRenderer {
    public:
        virtual void PushBlendMode(BlendMode mode) = 0;
        virtual void PopBlendMode() = 0;
        virtual void PushColor(Color const& color) = 0;
        virtual void PopColor() = 0;

        virtual void PushClip(IntRect const& rect) = 0;
        virtual void PopClip() = 0;

        virtual void RenderRect(IntRect const& rect) = 0;
        virtual void RenderFilledRect(IntRect const& rect) = 0;
        virtual void RenderImage(IImage& image, IntRect const& sourceRect, IntRect const& destRect, ImageScaleType scaleType, float scale) = 0;
        virtual void RenderText(std::string const& text, IFont& font, TextHorizAlignment horizontalAlignment, TextVertAlignment verticalAlignment, IntRect const& destRect) = 0;

        virtual void SetRendererLogicalSize(moth_ui::IntVec2 const& size) = 0;

        IRenderer() = default;
        IRenderer(IRenderer const&) = delete;
        IRenderer(IRenderer&&) = delete;
        IRenderer& operator=(IRenderer const&) = delete;
        IRenderer& operator=(IRenderer&&) = delete;
        virtual ~IRenderer() = default;
    };
}
