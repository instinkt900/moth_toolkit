#pragma once

#include "moth/graphics/graphics/igraphics.h"
#include "moth/graphics/graphics/color.h"
#include "moth/graphics/graphics/blend_mode.h"
#include "moth/graphics/utils/rect.h"

#include <moth/ui/graphics/irenderer.h>
#include <moth/ui/graphics/blend_mode.h>
#include <moth/ui/graphics/texture_filter.h>
#include <moth/ui/utils/color.h>
#include <moth/ui/utils/rect.h>

#include <stack>

namespace moth::bridge {
    /// @brief Adapts a moth::gfx IGraphics to the moth::ui IRenderer interface.
    class MothRenderer : public moth::ui::IRenderer {
    public:
        explicit MothRenderer(moth::gfx::IGraphics& graphics);
        ~MothRenderer() override = default;

        void PushBlendMode(moth::ui::BlendMode mode) override;
        void PopBlendMode() override;
        void PushColor(moth::ui::Color const& color) override;
        void PopColor() override;

        void PushTransform(moth::ui::FloatMat4x4 const& transform) override;
        void PopTransform() override;

        void PushClip(moth::ui::IntRect const& rect) override;
        void PopClip() override;

        void PushTextureFilter(moth::ui::TextureFilter filter) override;
        void PopTextureFilter() override;

        void RenderRect(moth::ui::IntRect const& rect) override;
        void RenderFilledRect(moth::ui::IntRect const& rect) override;
        void RenderGradientRect(moth::ui::IntRect const& rect, moth::ui::LinearGradient const& gradient) override;
        void RenderImage(moth::ui::IImage const& image, moth::ui::IntRect const& sourceRect, moth::ui::IntRect const& destRect, moth::ui::ImageScaleType scaleType, float scale) override;
        void RenderText(std::string_view text, moth::ui::IFont& font, moth::ui::TextHorizAlignment horizontalAlignment, moth::ui::TextVertAlignment verticalAlignment, moth::ui::IntRect const& destRect) override;

        void SetRendererLogicalSize(moth::ui::IntVec2 const& size) override;

    private:
        moth::gfx::IGraphics& m_graphics;
        std::stack<moth::gfx::Color> m_drawColor;
        std::stack<moth::ui::FloatMat4x4> m_transform;
        std::stack<moth::ui::TextureFilter> m_textureFilter;
    };
}
