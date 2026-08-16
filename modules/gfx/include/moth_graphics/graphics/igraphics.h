#pragma once

#include "moth_graphics/graphics/blend_mode.h"
#include "moth_graphics/graphics/color.h"
#include "moth_graphics/graphics/ifont.h"
#include "moth_graphics/graphics/image.h"
#include "moth_graphics/graphics/itarget.h"
#include "moth_graphics/graphics/shader.h"
#include "moth_graphics/graphics/text_alignment.h"
#include "moth_graphics/graphics/vertex.h"
#include "moth_graphics/utils/rect.h"
#include "moth_graphics/utils/transform.h"
#include "moth_graphics/utils/vector.h"

#include <cstddef>
#include <memory>

namespace moth::gfx::platform {
    class Window;
}

namespace moth::gfx {
    /// @brief Abstract 2D rendering interface.
    ///
    /// All drawing operations are batched between a @c Begin() / @c End() pair.
    /// State (color, blend mode, clip rect, render target) is set before issuing
    /// draw calls and remains active until changed.
    class IGraphics {
    public:
        virtual ~IGraphics() {}

        /// @brief Begin a new frame. Must be called before any draw operations.
        ///
        /// Always succeeds — if the swapchain is unavailable (e.g. window
        /// minimised), a null frame is started and all draw calls silently
        /// no-op until the next successful @c Begin().
        virtual void Begin() = 0;

        /// @brief End the current frame and present it.
        virtual void End() = 0;

        /// @brief Set the active blend mode for subsequent draw calls.
        /// @param mode The blend mode to apply.
        virtual void SetBlendMode(BlendMode mode) = 0;

        /// @brief Push a blend mode onto the state stack.
        ///
        /// Saves the current blend mode and activates @p mode until the matching
        /// @c PopBlendMode().
        virtual void PushBlendMode(BlendMode mode) = 0;

        /// @brief Restore the blend mode saved by the matching @c PushBlendMode.
        virtual void PopBlendMode() = 0;

        /// @brief Set the active draw color for subsequent draw calls.
        /// @param color The color to use (also modulates image draws).
        virtual void SetColor(Color const& color) = 0;

        /// @brief Push a draw color onto the state stack.
        ///
        /// Saves the current color and activates @p color until the matching
        /// @c PopColor().
        virtual void PushColor(Color const& color) = 0;

        /// @brief Restore the color saved by the matching @c PushColor.
        virtual void PopColor() = 0;

        /// @brief Set the active custom shader, or @c nullptr to restore the default.
        ///
        /// While a shader is active, every filled-shape draw (rects, circles,
        /// ellipses, polygons, triangles, lines) is rasterised with the shader's
        /// fragment program instead of the default one. The shader receives the
        /// interpolated vertex colour (via @c SetColor), the shape-local @c uv
        /// (0..1 across the shape's bounds), the Shadertoy built-ins
        /// (@c iTime/@c iResolution/@c iMouse), and the @c iChannel0..3 samplers.
        ///
        /// @c DrawImage (and @c DrawImageCircle) also honour the shader: the drawn
        /// image is bound as @c iChannel0, and @c uv carries its texture
        /// coordinates. @c DrawText is the exception — text always uses the font
        /// renderer and ignores the active shader.
        ///
        /// @param shader Shader to apply to subsequent draws, or @c nullptr to reset.
        virtual void SetShader(Shader const* shader) = 0;

        /// @brief Fill the entire render target with the current color.
        virtual void Clear() = 0;

        /// @brief Fill the entire render target with an explicit color.
        ///
        /// Unlike @c Clear(), this does not touch the active draw color, so a
        /// game can clear to a colour (e.g. sky) independent of the tint used
        /// for the draw calls that follow.
        /// @param color The color to clear with.
        virtual void Clear(Color const& color) = 0;

        /// @brief Set the active transform applied to all subsequent draw coordinates.
        /// @param transform Local-to-world transform for subsequent draw calls.
        virtual void SetTransform(FloatMat4x4 const& transform) = 0;

        /// @brief Pushes a transform that composes on top of the current transform.
        ///
        /// Subsequent draw coordinates are transformed by @c current * @p transform
        /// until @c PopTransform() restores the previous transform. This is how a
        /// camera transform (@c SetTransform) composes with a per-entity transform.
        virtual void PushTransform(FloatMat4x4 const& transform) = 0;

        /// @brief Restores the transform saved by the matching @c PushTransform.
        virtual void PopTransform() = 0;

        /// @brief Draw an image with a full 2D transform (position, rotation, scale) plus pivot and flip.
        ///
        /// The image is drawn at its natural size. @p transform places, rotates,
        /// and scales it; @p pivot (normalized) is the point within the image that
        /// lands on @p transform.position. The sprite's transform composes on top
        /// of the current transform (e.g. a camera view).
        ///
        /// @param image     The image to draw.
        /// @param transform Position/rotation/scale in local space.
        /// @param pivot     Normalized pivot: {0,0} = top-left, {0.5,0.5} = centre, {1,1} = bottom-right.
        /// @param flipX     Mirror the image horizontally.
        /// @param flipY     Mirror the image vertically.
        virtual void DrawImage(Image const& image, Transform2D const& transform,
                               FloatVec2 const& pivot = { 0.5f, 0.5f },
                               bool flipX = false, bool flipY = false) = 0;

        /// @brief Draw an image into a destination rectangle in local space. The active transform is applied.
        /// @param image The image to draw.
        /// @param destRect Destination rectangle in local (pre-transform) space.
        /// @param sourceRect Sub-region of the image to sample, or @c nullptr for the full image.
        virtual void DrawImage(Image const& image, IntRect const& destRect, IntRect const* sourceRect = nullptr) = 0;

        /// @brief Draw an image into a float destination rectangle (sub-pixel placement).
        /// @param image The image to draw.
        /// @param destRect Destination rectangle in local (pre-transform) space.
        /// @param sourceRect Sub-region of the image to sample, or @c nullptr for the full image.
        virtual void DrawImage(Image const& image, FloatRect const& destRect, IntRect const* sourceRect = nullptr) = 0;

        /// @brief Draw an image at a position, offset so that @p pivot within the image aligns with @p pos.
        /// @param image  The image to draw at natural size.
        /// @param pos    Destination point in logical pixels.
        /// @param pivot  Normalized pivot within the image: {0,0} = top-left, {0.5,0.5} = center,
        ///               {1,1} = bottom-right. Defaults to center.
        virtual void DrawImage(Image const& image, IntVec2 const& pos, FloatVec2 const& pivot = { 0.5f, 0.5f }) = 0;

        /// @brief Draw an image at a float position, offset so that @p pivot aligns with @p pos.
        /// @param image  The image to draw at natural size.
        /// @param pos    Destination point in logical pixels.
        /// @param pivot  Normalized pivot within the image, defaults to center.
        virtual void DrawImage(Image const& image, FloatVec2 const& pos, FloatVec2 const& pivot = { 0.5f, 0.5f }) = 0;

        /// @brief Tile an image to fill a destination rectangle.
        /// @param image The image to tile.
        /// @param destRect Destination rectangle in logical pixels.
        /// @param sourceRect Sub-region of the image to tile, or @c nullptr for the full image.
        /// @param scale Scale factor applied to each tile.
        virtual void DrawImageTiled(Image const& image, IntRect const& destRect, IntRect const* sourceRect = nullptr, float scale = 1.0f) = 0;

        /// @brief Tile an image to fill a float destination rectangle (sub-pixel placement).
        /// @param image The image to tile.
        /// @param destRect Destination rectangle in logical pixels.
        /// @param sourceRect Sub-region of the image to tile, or @c nullptr for the full image.
        /// @param scale Scale factor applied to each tile.
        virtual void DrawImageTiled(Image const& image, FloatRect const& destRect, IntRect const* sourceRect = nullptr, float scale = 1.0f) = 0;

        /// @brief Draw an axis-aligned rectangle outline using the current color.
        /// @param rect Rectangle in logical pixels.
        virtual void DrawRectF(FloatRect const& rect) = 0;

        /// @brief Draw a filled axis-aligned rectangle using the current color.
        /// @param rect Rectangle in logical pixels.
        virtual void DrawFillRectF(FloatRect const& rect) = 0;

        /// @brief Draw a filled circle using the current color.
        /// @param center Centre point in logical pixels (pre-transform).
        /// @param radius Radius in logical pixels.
        virtual void DrawFillCircleF(FloatVec2 const& center, float radius) = 0;

        /// @brief Draw a filled ellipse using the current color.
        /// @param center  Centre point in logical pixels (pre-transform).
        /// @param radiusX Horizontal radius in logical pixels.
        /// @param radiusY Vertical radius in logical pixels.
        virtual void DrawFillEllipseF(FloatVec2 const& center, float radiusX, float radiusY) = 0;

        /// @brief Draw a filled polygon using the current color.
        ///
        /// The polygon is given by its perimeter points in local (pre-transform)
        /// space, in either winding order; the active transform is applied. The
        /// outline must be simple (non-self-intersecting) but may be convex or
        /// concave. Holes are not supported. Fewer than three points draws
        /// nothing.
        ///
        /// Triangulation runs on every call, so this suits dynamic or
        /// modest-sized shapes. For large static geometry (e.g. land regions),
        /// triangulate once with @c moth::gfx::TriangulatePolygon and draw the
        /// cached vertices each frame via @c DrawTrianglesF instead.
        ///
        /// @param points Perimeter points in local pixels.
        /// @param count  Number of points.
        virtual void DrawFillPolygonF(FloatVec2 const* points, size_t count) = 0;

        /// @brief Draw a list of filled triangles using the current color.
        ///
        /// @p vertices holds three points per triangle in local (pre-transform)
        /// space; the active transform is applied. Any trailing points that do
        /// not complete a triple are ignored. This is the cached counterpart to
        /// @c DrawFillPolygonF: pair it with @c moth::gfx::TriangulatePolygon to
        /// triangulate a polygon once and redraw it without re-triangulating.
        ///
        /// @param vertices Triangle vertices in local pixels (three per triangle).
        /// @param count    Number of vertices.
        virtual void DrawTrianglesF(FloatVec2 const* vertices, size_t count) = 0;

        /// @brief Draw a list of textured triangles with per-vertex position, UV and color.
        ///
        /// The UVs are used directly (no bounding-box normalization), so this is
        /// the textured counterpart to @c DrawTrianglesF for sprite quads and
        /// arbitrary textured meshes. The active transform is applied to each
        /// vertex's position; the per-vertex color multiplies the texture sample.
        ///
        /// @param texture  Texture to sample.
        /// @param vertices Triangle vertices in local pixels (three per triangle).
        /// @param count    Number of vertices.
        virtual void DrawTexturedTrianglesF(ITexture& texture, TexturedVertex const* vertices, size_t count) = 0;

        /// @brief Draw a textured filled circle.
        ///
        /// The image is mapped across the circle's axis-aligned bounding box
        /// (@p center ± @p radius), with fragments outside the disc clipped by
        /// the tessellation. The current color modulates the texture sample,
        /// matching @c DrawImage tinting semantics.
        ///
        /// @param image      Source image; @c image.GetSourceRect() applies.
        /// @param center     Centre point in logical pixels (pre-transform).
        /// @param radius     Radius in logical pixels.
        /// @param sourceRect Sub-region of the image to sample, relative to
        ///                   the image's own origin, or @c nullptr for the
        ///                   full image.
        virtual void DrawImageCircle(Image const& image,
                                     FloatVec2 const& center,
                                     float radius,
                                     IntRect const* sourceRect = nullptr) = 0;

        /// @brief Border insets (in source pixels) for nine-slice drawing.
        struct NineSliceBorders {
            float left = 0.0f;
            float top = 0.0f;
            float right = 0.0f;
            float bottom = 0.0f;
        };

        /// @brief Draw an image nine-sliced into @p destRect.
        ///
        /// The image is split into a 3x3 grid by @p borders (in source pixels);
        /// the four corners are drawn at their border size while the edges and
        /// centre stretch to fill, so the borders are not distorted. If @p
        /// destRect is smaller than the borders, the borders shrink
        /// proportionally to fit.
        ///
        /// @param image    The image to draw.
        /// @param destRect Destination rectangle in logical pixels.
        /// @param borders  Source-space border insets for the slice.
        virtual void DrawImage9Slice(Image const& image, FloatRect const& destRect,
                                     NineSliceBorders const& borders) = 0;

        /// @brief Draw a linear gradient inside @p destRect.
        ///
        /// The transition runs along an axis at @p angle radians, centred at
        /// the midpoint (specified as a 0..1 factor of @p destRect), and lerps
        /// from @p startColor to @p endColor over a length of
        /// @p transitionLength × (the rect's projected extent along the
        /// gradient axis). Outside the transition band, colour is clamped.
        ///
        /// **Clip:** this call does NOT manage the scissor. Geometry outside
        /// @p destRect may be drawn unless the caller has set an appropriate
        /// clip — typically via @c SetClip in moth_graphics, or
        /// @c PushClip/PopClip when going through @c moth::ui::IRenderer.
        ///
        /// @param destRect          Destination rectangle in logical pixels.
        /// @param startColor        Colour at the start side of the axis.
        /// @param endColor          Colour at the end side of the axis.
        /// @param midpoint          Normalised 0..1 location of the t=0.5
        ///                          point inside @p destRect. (0.5, 0.5) =
        ///                          centre of the rect.
        /// @param angle             Direction of the gradient in radians;
        ///                          0 = +x (towards endColor side), π/2 = +y.
        /// @param transitionLength  Factor of the rect's projected extent
        ///                          along the gradient axis. 1.0 = fills the
        ///                          rect; 0.0 = sharp step at the midpoint;
        ///                          values > 1 leave the lerp partially
        ///                          outside the rect.
        virtual void DrawGradientRect(FloatRect const& destRect,
                                      Color startColor, Color endColor,
                                      FloatVec2 midpoint,
                                      float angle,
                                      float transitionLength) = 0;

        /// @brief Draw a line segment using the current color.
        /// @param p0 Start point in logical pixels.
        /// @param p1 End point in logical pixels.
        virtual void DrawLineF(FloatVec2 const& p0, FloatVec2 const& p1) = 0;

        /// @brief Draw a line segment of a given thickness using the current color.
        /// @param p0        Start point in logical pixels.
        /// @param p1        End point in logical pixels.
        /// @param thickness Line thickness in logical pixels.
        virtual void DrawLineF(FloatVec2 const& p0, FloatVec2 const& p1, float thickness) = 0;

        /// @brief Draw a string of text into a destination rectangle.
        ///
        /// Text always renders through the font renderer and ignores
        /// @c SetShader — the glyph atlas uses its own pipeline and descriptor
        /// layout that a custom fragment shader cannot describe.
        /// @param text UTF-8 text to render.
        /// @param font Font to use for rendering.
        /// @param destRect Bounding rectangle in logical pixels.
        /// @param horizontalAlignment Horizontal alignment within @p destRect.
        /// @param verticalAlignment Vertical alignment within @p destRect.
        virtual void DrawText(std::string_view text, IFont& font, IntRect const& destRect, TextHorizAlignment horizontalAlignment = TextHorizAlignment::Left, TextVertAlignment verticalAlignment = TextVertAlignment::Top) = 0;

        /// @brief Draw a Shadertoy-style shader across the whole logical viewport.
        ///
        /// The built-in uniforms @c iTime/@c iResolution/@c iMouse are filled
        /// automatically; @c iChannel0..3 come from @c Shader::SetChannel. Useful
        /// as a fullscreen post-processing pass over a render target.
        /// @param shader The shader to run.
        virtual void DrawShader(Shader const& shader) = 0;

        /// @brief Draw a Shadertoy-style shader inside @p destRect (logical pixels).
        ///
        /// The shader's @c fragCoord spans @p destRect (origin at the rect's
        /// top-left); the active transform still applies to the quad's placement.
        /// @param shader   The shader to run.
        /// @param destRect Destination rectangle in logical pixels.
        virtual void DrawShader(Shader const& shader, FloatRect const& destRect) = 0;

        /// @brief Set the scissor clip rectangle. Pass @c nullptr to clear clipping.
        /// @param rect Clip rectangle in logical pixels, or @c nullptr to disable.
        virtual void SetClip(IntRect const* rect) = 0;

        /// @brief Push a clip rectangle, intersected with the current clip.
        ///
        /// Subsequent draws are clipped to the intersection of @p rect and any
        /// enclosing clip, forming a nested clip stack. The previous clip is
        /// restored by @c PopClip().
        /// @param rect Clip rectangle in logical pixels.
        virtual void PushClip(IntRect const& rect) = 0;

        /// @brief Restore the clip saved by the matching @c PushClip.
        virtual void PopClip() = 0;

        /// @brief Returns the currently active render target, or @c nullptr if rendering to the swapchain.
        virtual ITarget* GetTarget() = 0;

        /// @brief Set the active render target. Pass @c nullptr to restore rendering to the swapchain.
        ///
        /// Render targets themselves are created by @c IGraphicsDevice::CreateTarget.
        /// @param target Render target to draw into, or @c nullptr.
        virtual void SetTarget(ITarget* target) = 0;

        /// @brief Override the logical rendering resolution used to map draw coordinates.
        /// @param logicalSize Logical width and height in pixels.
        virtual void SetLogicalSize(IntVec2 const& logicalSize) = 0;

    };
}
