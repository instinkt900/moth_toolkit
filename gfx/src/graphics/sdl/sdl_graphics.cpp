#include "common.h"
#include "sdl_graphics.h"
#include "sdl_font.h"
#include "sdl_target.h"
#include "sdl_texture.h"
#include "sdl_utils.hpp"
#include "moth_graphics/platform/sdl/sdl_window.h"
#include "../utils.h"

#include "moth_graphics/utils/circle_tessellation.h"

namespace moth_graphics::graphics::sdl {
    Graphics::Graphics(SurfaceContext& context)
        : m_surfaceContext(context)
        , m_drawColor(graphics::BasicColors::White) {
    }

    Graphics::~Graphics() = default;

    void Graphics::Begin() {
        // Clear the full physical window with black before any logical-size
        // letterboxing kicks in. With a logical size set, SDL_RenderClear only
        // clears the logical viewport, leaving the letterbox bars uninitialised.
        SDL_RenderSetLogicalSize(m_surfaceContext.GetRenderer(), 0, 0);
        SDL_SetRenderDrawColor(m_surfaceContext.GetRenderer(), 0, 0, 0, 255);
        SDL_RenderClear(m_surfaceContext.GetRenderer());
        // Restore the renderer's draw color so subsequent primitive draws
        // honour the cached m_drawColor instead of inheriting black.
        ColorComponents const components(m_drawColor);
        SDL_SetRenderDrawColor(m_surfaceContext.GetRenderer(), components.r, components.g, components.b, components.a);
    }

    void Graphics::End() {
    }

    void Graphics::SetBlendMode(graphics::BlendMode mode) {
        SDL_SetRenderDrawBlendMode(m_surfaceContext.GetRenderer(), ToSDL(mode));
        m_blendMode = mode;
    }

    void Graphics::SetColor(graphics::Color const& color) {
        ColorComponents components(color);
        SDL_SetRenderDrawColor(m_surfaceContext.GetRenderer(), components.r, components.g, components.b, components.a);
        m_drawColor = color;
    }

    void Graphics::Clear() {
        SDL_RenderClear(m_surfaceContext.GetRenderer());
    }

    FloatMat4x4 Graphics::CurrentTransform() const {
        return m_currentTransform;
    }

    void Graphics::SetTransform(FloatMat4x4 const& transform) {
        m_currentTransform = transform;
    }

    void Graphics::DrawImage(Image const& image, IntVec2 const& pos, FloatVec2 const& pivot) {
        auto const imageWidth = image.GetWidth();
        auto const imageHeight = image.GetHeight();
        auto const offsetX = static_cast<int>(static_cast<float>(imageWidth) * pivot.x);
        auto const offsetY = static_cast<int>(static_cast<float>(imageHeight) * pivot.y);
        IntRect destRect = MakeRect(pos.x, pos.y, imageWidth, imageHeight);
        DrawImage(image, destRect - IntVec2{ offsetX, offsetY }, nullptr);
    }

    void Graphics::DrawImage(Image const& image, IntRect const& destRect, IntRect const* sourceRect) {
        auto sdlTexture = std::dynamic_pointer_cast<Texture>(image.GetTexture());
        if (!sdlTexture) {
            return;
        }
        auto const& textureSourceRect = image.GetSourceRect();

        ColorComponents const components{ m_drawColor };
        SDL_SetTextureBlendMode(sdlTexture->GetSDLTexture()->GetImpl(), ToSDL(m_blendMode));
        SDL_SetTextureColorMod(sdlTexture->GetSDLTexture()->GetImpl(), components.r, components.g, components.b);
        SDL_SetTextureAlphaMod(sdlTexture->GetSDLTexture()->GetImpl(), components.a);

        SDL_Rect sdlSrcRect = ToSDL(sourceRect != nullptr ? *sourceRect : textureSourceRect);

        // Negative src dimensions mean the caller wants a mirrored draw.
        // SDL doesn't support negative-dimension rects; normalize and flip instead.
        bool flipH = false;
        bool flipV = false;
        if (sdlSrcRect.w < 0) {
            sdlSrcRect.x += sdlSrcRect.w;
            sdlSrcRect.w = -sdlSrcRect.w;
            flipH = true;
        }
        if (sdlSrcRect.h < 0) {
            sdlSrcRect.y += sdlSrcRect.h;
            sdlSrcRect.h = -sdlSrcRect.h;
            flipV = true;
        }
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (flipH && flipV) {
            flip = static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        } else if (flipH) {
            flip = SDL_FLIP_HORIZONTAL;
        } else if (flipV) {
            flip = SDL_FLIP_VERTICAL;
        }

        auto const t = CurrentTransform();
        float const localCenterX = static_cast<float>(destRect.topLeft.x + destRect.bottomRight.x) * 0.5f;
        float const localCenterY = static_cast<float>(destRect.topLeft.y + destRect.bottomRight.y) * 0.5f;
        auto const worldCenter = t.TransformPoint({ localCenterX, localCenterY });
        float const w = static_cast<float>(destRect.bottomRight.x - destRect.topLeft.x);
        float const h = static_cast<float>(destRect.bottomRight.y - destRect.topLeft.y);
        SDL_Rect const sdlDstRect{
            static_cast<int>(worldCenter.x - (w * 0.5f)),
            static_cast<int>(worldCenter.y - (h * 0.5f)),
            static_cast<int>(w),
            static_cast<int>(h),
        };
        double const rotation = static_cast<double>(t.GetRotationDegrees());

        SDL_RenderCopyEx(m_surfaceContext.GetRenderer(),
                         sdlTexture->GetSDLTexture()->GetImpl(),
                         &sdlSrcRect,
                         &sdlDstRect,
                         rotation,
                         nullptr,
                         flip);
    }

    void Graphics::DrawImageTiled(Image const& image, IntRect const& destRect, IntRect const* sourceRect, float scale) {
        auto const imageWidth = static_cast<int>(static_cast<float>(image.GetWidth()) * scale);
        auto const imageHeight = static_cast<int>(static_cast<float>(image.GetHeight()) * scale);
        if (imageWidth <= 0 || imageHeight <= 0) {
            return;
        }
        for (auto y = destRect.topLeft.y; y < destRect.bottomRight.y; y += imageHeight) {
            for (auto x = destRect.topLeft.x; x < destRect.bottomRight.x; x += imageWidth) {
                DrawImage(image, IntRect{ { x, y }, { x + imageWidth, y + imageHeight } }, sourceRect);
            }
        }
    }

    void Graphics::DrawRectF(FloatRect const& rect) {
        auto const t = CurrentTransform();
        auto const tl = t.TransformPoint({ rect.topLeft.x, rect.topLeft.y });
        auto const tr = t.TransformPoint({ rect.bottomRight.x, rect.topLeft.y });
        auto const br = t.TransformPoint({ rect.bottomRight.x, rect.bottomRight.y });
        auto const bl = t.TransformPoint({ rect.topLeft.x, rect.bottomRight.y });
        SDL_FPoint const pts[5] = {
            { tl.x, tl.y },
            { tr.x, tr.y },
            { br.x, br.y },
            { bl.x, bl.y },
            { tl.x, tl.y },
        };
        SDL_RenderDrawLinesF(m_surfaceContext.GetRenderer(), pts, 5);
    }

    void Graphics::DrawFillRectF(FloatRect const& rect) {
        auto const t = CurrentTransform();
        auto const tl = t.TransformPoint({ rect.topLeft.x, rect.topLeft.y });
        auto const tr = t.TransformPoint({ rect.bottomRight.x, rect.topLeft.y });
        auto const br = t.TransformPoint({ rect.bottomRight.x, rect.bottomRight.y });
        auto const bl = t.TransformPoint({ rect.topLeft.x, rect.bottomRight.y });
        ColorComponents const components{ m_drawColor };
        SDL_Color const sdlColor{ components.r, components.g, components.b, components.a };
        SDL_Vertex const sdlVertices[6] = {
            { { tl.x, tl.y }, sdlColor, { 0.0f, 0.0f } },
            { { tr.x, tr.y }, sdlColor, { 0.0f, 0.0f } },
            { { bl.x, bl.y }, sdlColor, { 0.0f, 0.0f } },
            { { tr.x, tr.y }, sdlColor, { 0.0f, 0.0f } },
            { { br.x, br.y }, sdlColor, { 0.0f, 0.0f } },
            { { bl.x, bl.y }, sdlColor, { 0.0f, 0.0f } },
        };
        SDL_RenderGeometry(m_surfaceContext.GetRenderer(), nullptr, sdlVertices, 6, nullptr, 0);
    }

    void Graphics::DrawFillCircleF(FloatVec2 const& center, float radius) {
        if (radius <= 0.0f) {
            return;
        }
        int const segments = detail::CircleSegmentCount(radius);
        auto const t = CurrentTransform();
        auto const centerW = t.TransformPoint(center);
        ColorComponents const comp{ m_drawColor };
        SDL_Color const sdlColor{ comp.r, comp.g, comp.b, comp.a };
        constexpr float kTwoPi = 6.28318530718f;

        std::vector<SDL_Vertex> vertices(static_cast<size_t>(segments) * 3);
        FloatVec2 prev = t.TransformPoint({ center.x + radius, center.y });
        for (int i = 0; i < segments; ++i) {
            float const a = (kTwoPi * static_cast<float>(i + 1)) / static_cast<float>(segments);
            FloatVec2 const next = t.TransformPoint({
                center.x + (std::cos(a) * radius),
                center.y + (std::sin(a) * radius),
            });
            auto const base = static_cast<size_t>(i) * 3;
            vertices[base + 0] = { { centerW.x, centerW.y }, sdlColor, { 0.0f, 0.0f } };
            vertices[base + 1] = { { prev.x,    prev.y    }, sdlColor, { 0.0f, 0.0f } };
            vertices[base + 2] = { { next.x,    next.y    }, sdlColor, { 0.0f, 0.0f } };
            prev = next;
        }
        SDL_RenderGeometry(m_surfaceContext.GetRenderer(), nullptr,
                           vertices.data(), static_cast<int>(vertices.size()),
                           nullptr, 0);
    }

    void Graphics::DrawImageCircle(Image const& image, FloatVec2 const& center, float radius, IntRect const* sourceRect) {
        if (radius <= 0.0f) {
            return;
        }
        auto sdlTexture = std::dynamic_pointer_cast<Texture>(image.GetTexture());
        if (!sdlTexture) {
            return;
        }

        FloatRect imageRect;
        if (sourceRect != nullptr) {
            imageRect = static_cast<FloatRect>(*sourceRect);
        } else {
            imageRect = MakeRect(0.0f, 0.0f, static_cast<float>(image.GetWidth()), static_cast<float>(image.GetHeight()));
        }
        FloatVec2 const textureDimensions{
            static_cast<float>(sdlTexture->GetWidth()),
            static_cast<float>(sdlTexture->GetHeight()),
        };
        imageRect += static_cast<FloatVec2>(image.GetSourceRect().topLeft);
        imageRect /= textureDimensions;

        // Drive tint + alpha + blend through the texture state, matching the
        // SDL DrawImage path. Vertex colors stay white so they don't double up.
        SDL_Texture* const sdlTex = sdlTexture->GetSDLTexture()->GetImpl();
        ColorComponents const comp{ m_drawColor };
        SDL_SetTextureBlendMode(sdlTex, ToSDL(m_blendMode));
        SDL_SetTextureColorMod(sdlTex, comp.r, comp.g, comp.b);
        SDL_SetTextureAlphaMod(sdlTex, comp.a);
        SDL_Color const sdlWhite{ 255, 255, 255, 255 };

        int const segments = detail::CircleSegmentCount(radius);
        auto const t = CurrentTransform();
        constexpr float kTwoPi = 6.28318530718f;

        auto computeUv = [&](float lx, float ly) -> FloatVec2 {
            float const u = (lx - (center.x - radius)) / (2.0f * radius);
            float const v = (ly - (center.y - radius)) / (2.0f * radius);
            return {
                imageRect.topLeft.x + (u * (imageRect.bottomRight.x - imageRect.topLeft.x)),
                imageRect.topLeft.y + (v * (imageRect.bottomRight.y - imageRect.topLeft.y)),
            };
        };

        auto const centerW = t.TransformPoint(center);
        FloatVec2 const centerUv = computeUv(center.x, center.y);

        std::vector<SDL_Vertex> vertices(static_cast<size_t>(segments) * 3);
        float prevLx = center.x + radius;
        float prevLy = center.y;
        FloatVec2 prevW = t.TransformPoint({ prevLx, prevLy });
        FloatVec2 prevUv = computeUv(prevLx, prevLy);
        for (int i = 0; i < segments; ++i) {
            float const a = (kTwoPi * static_cast<float>(i + 1)) / static_cast<float>(segments);
            float const nextLx = center.x + (std::cos(a) * radius);
            float const nextLy = center.y + (std::sin(a) * radius);
            FloatVec2 const nextW = t.TransformPoint({ nextLx, nextLy });
            FloatVec2 const nextUv = computeUv(nextLx, nextLy);
            auto const base = static_cast<size_t>(i) * 3;
            vertices[base + 0] = { { centerW.x, centerW.y }, sdlWhite, { centerUv.x, centerUv.y } };
            vertices[base + 1] = { { prevW.x,   prevW.y   }, sdlWhite, { prevUv.x,   prevUv.y   } };
            vertices[base + 2] = { { nextW.x,   nextW.y   }, sdlWhite, { nextUv.x,   nextUv.y   } };
            prevW = nextW;
            prevUv = nextUv;
        }

        SDL_RenderGeometry(m_surfaceContext.GetRenderer(), sdlTex,
                           vertices.data(), static_cast<int>(vertices.size()),
                           nullptr, 0);
    }

    void Graphics::DrawGradientRect(FloatRect const& destRect,
                                    Color startColor, Color endColor,
                                    FloatVec2 midpoint,
                                    float angle,
                                    float transitionLength) {
        float const w = destRect.bottomRight.x - destRect.topLeft.x;
        float const h = destRect.bottomRight.y - destRect.topLeft.y;
        if (w <= 0.0f || h <= 0.0f) {
            return;
        }

        // Pixel-space midpoint, gradient axis direction + perpendicular.
        FloatVec2 const mp{
            destRect.topLeft.x + (midpoint.x * w),
            destRect.topLeft.y + (midpoint.y * h),
        };
        float const c = std::cos(angle);
        float const s = std::sin(angle);
        FloatVec2 const dir{ c, s };
        FloatVec2 const perp{ -s, c };

        // Rect's extent projected onto the gradient axis. This is what
        // transitionLength is a factor of, so a value of 1.0 fills the rect
        // along the axis for any angle.
        float const projExtent = (std::abs(w * c) + std::abs(h * s));
        float const transitionPixels = std::max(0.0f, transitionLength) * projExtent;
        float const halfL = transitionPixels * 0.5f;

        // Perpendicular extent large enough that the rotated band covers the
        // entire dest rect regardless of angle. The scissor (set by the
        // caller, or by moth_ui::IRenderer::RenderGradientRect) trims the
        // visible portion to destRect.
        float const cover = std::sqrt((w * w) + (h * h));

        auto const t = CurrentTransform();
        auto toWorld = [&](float lx, float ly) {
            FloatVec2 const local{
                mp.x + (dir.x * lx) + (perp.x * ly),
                mp.y + (dir.y * lx) + (perp.y * ly),
            };
            return t.TransformPoint(local);
        };

        ColorComponents const startComp{ startColor };
        ColorComponents const endComp{ endColor };
        SDL_Color const sdlStart{ startComp.r, startComp.g, startComp.b, startComp.a };
        SDL_Color const sdlEnd{ endComp.r, endComp.g, endComp.b, endComp.a };

        auto submitQuad = [&](float x0, float x1, SDL_Color c0, SDL_Color c1) {
            if (x0 >= x1) {
                return;
            }
            auto const tl = toWorld(x0, -cover);
            auto const tr = toWorld(x1, -cover);
            auto const bl = toWorld(x0, +cover);
            auto const br = toWorld(x1, +cover);
            SDL_Vertex const verts[6] = {
                { { tl.x, tl.y }, c0, { 0.0f, 0.0f } },
                { { tr.x, tr.y }, c1, { 0.0f, 0.0f } },
                { { bl.x, bl.y }, c0, { 0.0f, 0.0f } },
                { { tr.x, tr.y }, c1, { 0.0f, 0.0f } },
                { { br.x, br.y }, c1, { 0.0f, 0.0f } },
                { { bl.x, bl.y }, c0, { 0.0f, 0.0f } },
            };
            SDL_RenderGeometry(m_surfaceContext.GetRenderer(), nullptr, verts, 6, nullptr, 0);
        };

        // Left flank (clamped to startColor) → transition band → right flank
        // (clamped to endColor). If transitionPixels == 0 the middle is
        // skipped and we get a sharp step where the two flanks meet.
        submitQuad(-cover, -halfL, sdlStart, sdlStart);
        submitQuad(-halfL, +halfL, sdlStart, sdlEnd);
        submitQuad(+halfL, +cover, sdlEnd, sdlEnd);
    }

    void Graphics::DrawLineF(FloatVec2 const& p0, FloatVec2 const& p1) {
        auto const t = CurrentTransform();
        auto const wp0 = t.TransformPoint(p0);
        auto const wp1 = t.TransformPoint(p1);
        SDL_RenderDrawLineF(m_surfaceContext.GetRenderer(), wp0.x, wp0.y, wp1.x, wp1.y);
    }

    void Graphics::DrawText(std::string_view text, graphics::IFont& font, IntRect const& destRect, graphics::TextHorizAlignment horizontalAlignment, graphics::TextVertAlignment verticalAlignment) {
        std::string const textStr(text);
        auto* sdlFont = dynamic_cast<Font*>(&font);
        if (sdlFont == nullptr) {
            spdlog::warn("DrawText: font is not an SDL Font; skipping");
            return;
        }
        auto const fcFont = sdlFont->GetFontObj();

        auto const destWidth = destRect.bottomRight.x - destRect.topLeft.x;
        auto const destHeight = destRect.bottomRight.y - destRect.topLeft.y;
        auto const textHeight = FC_GetColumnHeight(fcFont.get(), destWidth, "%s", textStr.c_str()); // NOLINT(cppcoreguidelines-pro-type-vararg)

        // local x/y: position within destRect for the text origin, accounting for alignment
        auto x = 0.0f;
        switch (horizontalAlignment) {
        case graphics::TextHorizAlignment::Left:
            break;
        case graphics::TextHorizAlignment::Center:
            x = static_cast<float>(destWidth) / 2.0f;
            break;
        case graphics::TextHorizAlignment::Right:
            x = static_cast<float>(destWidth);
            break;
        }

        auto y = 0.0f;
        switch (verticalAlignment) {
        case graphics::TextVertAlignment::Top:
            break;
        case graphics::TextVertAlignment::Middle:
            y = (static_cast<float>(destHeight) - static_cast<float>(textHeight)) / 2.0f;
            break;
        case graphics::TextVertAlignment::Bottom:
            y = static_cast<float>(destHeight) - static_cast<float>(textHeight);
            break;
        }

        FC_Effect effect;
        effect.alignment = ToSDL(horizontalAlignment);
        effect.color = ToSDL(m_drawColor);
        effect.scale.x = 1.0f;
        effect.scale.y = 1.0f;

        auto const t = CurrentTransform();
        double const rotation = static_cast<double>(t.GetRotationDegrees());

        if (rotation == 0.0) {
            // Fast path: no rotation — draw directly at the transformed world position.
            auto const worldPos = t.TransformPoint({ static_cast<float>(destRect.topLeft.x) + x,
                                                     static_cast<float>(destRect.topLeft.y) + y });
            FC_DrawColumnEffect(fcFont.get(), m_surfaceContext.GetRenderer(), worldPos.x, worldPos.y, destWidth, effect, "%s", textStr.c_str()); // NOLINT(cppcoreguidelines-pro-type-vararg)
        } else {
            // Rotation path: render text to a scratch texture, then draw it rotated.
            // Grow the scratch texture only when the current one is too small.
            if (!m_textScratchTexture || destWidth > m_textScratchWidth || destHeight > m_textScratchHeight) {
                m_textScratchWidth = std::max(destWidth, m_textScratchWidth);
                m_textScratchHeight = std::max(destHeight, m_textScratchHeight);
                m_textScratchTexture = CreateTextureRef(SDL_CreateTexture(m_surfaceContext.GetRenderer(),
                                                                          SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_textScratchWidth, m_textScratchHeight));
                SDL_SetTextureBlendMode(m_textScratchTexture->GetImpl(), SDL_BLENDMODE_BLEND);
            }

            SDL_Texture* prevTarget = SDL_GetRenderTarget(m_surfaceContext.GetRenderer());
            SDL_SetRenderTarget(m_surfaceContext.GetRenderer(), m_textScratchTexture->GetImpl());
            SDL_SetRenderDrawColor(m_surfaceContext.GetRenderer(), 0, 0, 0, 0);
            SDL_RenderClear(m_surfaceContext.GetRenderer());

            FC_DrawColumnEffect(fcFont.get(), m_surfaceContext.GetRenderer(), x, y, destWidth, effect, "%s", textStr.c_str()); // NOLINT(cppcoreguidelines-pro-type-vararg)

            SDL_SetRenderTarget(m_surfaceContext.GetRenderer(), prevTarget);

            // Draw the scratch texture at the world-space center of destRect, rotated.
            float const localCenterX = static_cast<float>(destWidth) * 0.5f;
            float const localCenterY = static_cast<float>(destHeight) * 0.5f;
            auto const worldCenter = t.TransformPoint({ static_cast<float>(destRect.topLeft.x) + localCenterX,
                                                        static_cast<float>(destRect.topLeft.y) + localCenterY });
            SDL_Rect const srcRect{ 0, 0, destWidth, destHeight };
            SDL_Rect const dstRect{
                static_cast<int>(worldCenter.x - localCenterX),
                static_cast<int>(worldCenter.y - localCenterY),
                destWidth,
                destHeight,
            };
            SDL_RenderCopyEx(m_surfaceContext.GetRenderer(), m_textScratchTexture->GetImpl(),
                             &srcRect, &dstRect, rotation, nullptr, SDL_FLIP_NONE);
        }
    }

    void Graphics::SetClip(IntRect const* rect) {
        if (rect != nullptr) {
            auto const currentRect = ToSDL(*rect);
            SDL_RenderSetClipRect(m_surfaceContext.GetRenderer(), &currentRect);
        } else {
            SDL_RenderSetClipRect(m_surfaceContext.GetRenderer(), nullptr);
        }
    }

    std::unique_ptr<graphics::ITarget> Graphics::CreateTarget(int width, int height) {
        auto sdlTexture = CreateTextureRef(SDL_CreateTexture(m_surfaceContext.GetRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height));
        auto texture = std::make_shared<Texture>(m_surfaceContext.GetRenderer(), sdlTexture);
        return std::make_unique<Target>(std::move(texture));
    }

    graphics::ITarget* Graphics::GetTarget() {
        return m_currentRenderTarget;
    }

    void Graphics::SetTarget(graphics::ITarget* target) {
        if (target == nullptr) {
            SDL_SetRenderTarget(m_surfaceContext.GetRenderer(), nullptr);
        } else {
            auto* sdlTarget = dynamic_cast<Target*>(target);
            assert(sdlTarget != nullptr && "SetTarget: target is not an SDL Target");
            auto const& sdlTexture = sdlTarget->GetSDLTexture()->GetSDLTexture();
            SDL_SetRenderTarget(m_surfaceContext.GetRenderer(), sdlTexture->GetImpl());
        }
        m_currentRenderTarget = target;
    }

    void Graphics::SetLogicalSize(IntVec2 const& logicalSize) {
        SDL_RenderSetLogicalSize(m_surfaceContext.GetRenderer(), logicalSize.x, logicalSize.y);
    }

    std::unique_ptr<IGraphics> CreateGraphics(SurfaceContext& surfaceContext) {
        return std::make_unique<Graphics>(surfaceContext);
    }
}
