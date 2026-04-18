#pragma once

#include "moth_ui/moth_ui_fwd.h"
#include "moth_ui/graphics/blend_mode.h"
#include "moth_ui/graphics/image_scale_type.h"
#include "moth_ui/graphics/text_alignment.h"
#include "moth_ui/graphics/texture_filter.h"
#include "moth_ui/utils/color.h"
#include "moth_ui/utils/rect.h"
#include "moth_ui/utils/transform.h"

#include <string_view>

namespace moth_ui {
    /**
     * @brief Abstract rendering interface that the UI system draws through.
     *
     * Backend implementations (SDL2, Vulkan, …) provide a concrete subclass.
     * State is managed via a stack model: Push sets state, Pop restores it.
     */
    class IRenderer {
    public:
        /**
         * @brief Pushes a blend mode onto the blend-mode stack.
         * @param mode The blend mode to activate.
         */
        virtual void PushBlendMode(BlendMode mode) = 0;

        /// @brief Pops the top blend mode, restoring the previous one.
        virtual void PopBlendMode() = 0;

        /**
         * @brief Pushes a colour modulation value onto the colour stack.
         * @param color The colour to multiply draw calls by.
         */
        virtual void PushColor(Color const& color) = 0;

        /// @brief Pops the top colour, restoring the previous one.
        virtual void PopColor() = 0;

        /**
         * @brief Pushes a 2D affine transform. Draw calls use local-space rects;
         *        the renderer maps them to screen space via this transform.
         *        Transforms are not composed — each push replaces the active transform
         *        until the matching PopTransform.
         * @param transform The absolute local-to-screen transform for the current node.
         */
        virtual void PushTransform(FloatMat4x4 const& transform) = 0;

        /// @brief Pops the current transform, restoring the previous one.
        virtual void PopTransform() = 0;

        /**
         * @brief Pushes a clip rectangle; subsequent draws are clipped to it.
         * @param rect The scissor rectangle in screen space.
         */
        virtual void PushClip(IntRect const& rect) = 0;

        /// @brief Pops the top clip rectangle, restoring the previous clip region.
        virtual void PopClip() = 0;

        /**
         * @brief Pushes a texture sampling filter onto the filter stack.
         *
         * Affects subsequent @c RenderImage calls until the matching @c PopTextureFilter.
         * The default (bottom of stack) is @c TextureFilter::Linear.
         * @param filter The filter to apply.
         */
        virtual void PushTextureFilter(TextureFilter filter) = 0;

        /// @brief Pops the top texture filter, restoring the previous one.
        virtual void PopTextureFilter() = 0;

        /**
         * @brief Draws the outline of a rectangle using the current colour.
         * @param rect Destination rectangle in screen space.
         */
        virtual void RenderRect(IntRect const& rect) = 0;

        /**
         * @brief Draws a filled rectangle using the current colour.
         * @param rect Destination rectangle in screen space.
         */
        virtual void RenderFilledRect(IntRect const& rect) = 0;

        /**
         * @brief Draws a portion of an image into a destination rectangle.
         * @param image      Source image to draw.
         * @param sourceRect Region of the source image to sample.
         * @param destRect   Destination rectangle in screen space.
         * @param scaleType  How the image is scaled to fill @p destRect.
         * @param scale      Uniform scale factor applied during tiled/nine-slice modes.
         */
        virtual void RenderImage(IImage const& image, IntRect const& sourceRect, IntRect const& destRect, ImageScaleType scaleType, float scale) = 0;

        /**
         * @brief Draws a UTF-8 string using the specified font and alignment.
         * @param text                The string to render.
         * @param font                Font to render the text with.
         * @param horizontalAlignment Horizontal alignment within @p destRect.
         * @param verticalAlignment   Vertical alignment within @p destRect.
         * @param destRect            Bounding rectangle in screen space.
         */
        virtual void RenderText(std::string_view text, IFont& font, TextHorizAlignment horizontalAlignment, TextVertAlignment verticalAlignment, IntRect const& destRect) = 0;

        /**
         * @brief Sets the logical (virtual) rendering resolution.
         * @param size Width and height in logical pixels.
         */
        virtual void SetRendererLogicalSize(moth_ui::IntVec2 const& size) = 0;

        IRenderer() = default;
        IRenderer(IRenderer const&) = default;
        IRenderer(IRenderer&&) = default;
        IRenderer& operator=(IRenderer const&) = default;
        IRenderer& operator=(IRenderer&&) = default;
        virtual ~IRenderer() = default;
    };
}
