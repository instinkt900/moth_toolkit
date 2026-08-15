#include "moth/bridge/moth_renderer.h"
#include "moth_graphics/graphics/igraphics.h"
#include "moth/bridge/moth_font.h"
#include "moth/bridge/moth_image.h"

#include <algorithm>
#include <cmath>

namespace moth::bridge {
    using namespace moth::gfx::graphics;
    using namespace moth::core;
    namespace graphics = moth::gfx::graphics;
    namespace {
        TextureFilter ToGraphicsFilter(moth::ui::TextureFilter f) {
            return f == moth::ui::TextureFilter::Nearest
                ? TextureFilter::Nearest
                : TextureFilter::Linear;
        }
    }

    MothRenderer::MothRenderer(IGraphics& graphics)
        : m_graphics(graphics) {
        m_drawColor.push({ 1.0f, 1.0f, 1.0f, 1.0f });
        m_blendMode.push(BlendMode::Replace);
        m_transform.push(moth::ui::FloatMat4x4::Identity());
        m_graphics.SetTransform(m_transform.top());
        m_textureFilter.push(moth::ui::TextureFilter::Linear);
    }

    void MothRenderer::PushBlendMode(moth::ui::BlendMode mode) {
        m_blendMode.push(mode);
    }

    void MothRenderer::PopBlendMode() {
        if (m_blendMode.size() > 1) {
            m_blendMode.pop();
        }
    }

    void MothRenderer::PushColor(moth::ui::Color const& color) {
        auto const modColor = m_drawColor.top() * color;
        m_drawColor.push(modColor);
    }

    void MothRenderer::PopColor() {
        if (m_drawColor.size() > 1) {
            m_drawColor.pop();
        }
    }

    IntRect ClipRect(IntRect const& parentRect, IntRect const& childRect) {
        IntRect result;
        result.topLeft.x = std::max(parentRect.topLeft.x, childRect.topLeft.x);
        result.topLeft.y = std::max(parentRect.topLeft.y, childRect.topLeft.y);
        result.bottomRight.x = std::min(parentRect.bottomRight.x, childRect.bottomRight.x);
        result.bottomRight.y = std::min(parentRect.bottomRight.y, childRect.bottomRight.y);
        result.bottomRight.x = std::max(result.topLeft.x, result.bottomRight.x);
        result.bottomRight.y = std::max(result.topLeft.y, result.bottomRight.y);
        return result;
    }

    void MothRenderer::PushTransform(moth::ui::FloatMat4x4 const& transform) {
        m_transform.push(transform);
        m_graphics.SetTransform(transform);
    }

    void MothRenderer::PopTransform() {
        if (m_transform.size() > 1) {
            m_transform.pop();
            m_graphics.SetTransform(m_transform.top());
        }
    }

    void MothRenderer::PushClip(moth::ui::IntRect const& rect) {
        if (m_clip.empty()) {
            m_clip.push(rect);
        } else {
            // want to clip rect within the current clip
            auto const parentRect = m_clip.top();
            auto const newRect = ClipRect(parentRect, rect);
            m_clip.push(newRect);
        }

        m_graphics.SetClip(&m_clip.top());
    }

    void MothRenderer::PopClip() {
        if (m_clip.empty()) {
            return;
        }
        m_clip.pop();

        if (m_clip.empty()) {
            m_graphics.SetClip(nullptr);
        } else {
            m_graphics.SetClip(&m_clip.top());
        }
    }

    void MothRenderer::PushTextureFilter(moth::ui::TextureFilter filter) {
        m_textureFilter.push(filter);
    }

    void MothRenderer::PopTextureFilter() {
        if (m_textureFilter.size() > 1) {
            m_textureFilter.pop();
        }
    }

    void MothRenderer::RenderRect(moth::ui::IntRect const& rect) {
        m_graphics.SetBlendMode(m_blendMode.top());
        m_graphics.SetColor(m_drawColor.top());
        m_graphics.DrawRectF(static_cast<FloatRect>(rect));
    }

    void MothRenderer::RenderFilledRect(moth::ui::IntRect const& rect) {
        m_graphics.SetBlendMode(m_blendMode.top());
        m_graphics.SetColor(m_drawColor.top());
        m_graphics.DrawFillRectF(static_cast<FloatRect>(rect));
    }

    void MothRenderer::RenderGradientRect(moth::ui::IntRect const& rect, moth::ui::LinearGradient const& gradient) {
        m_graphics.SetBlendMode(m_blendMode.top());
        // Modulate gradient stop colours by the current renderer colour stack so
        // ancestor SetColor / alpha overrides compose with the gradient the same
        // way they do with RenderFilledRect.
        moth::ui::Color const tint = m_drawColor.top();

        // The backend gradient implementation rasterises rotated quads that may
        // extend beyond the destination rect. Push a scissor matching the rect's
        // screen-space AABB so the visible gradient stays inside the node.
        auto const& transform = m_transform.top();
        moth::ui::FloatVec2 const corners[4] = {
            transform.TransformPoint({ static_cast<float>(rect.topLeft.x),     static_cast<float>(rect.topLeft.y) }),
            transform.TransformPoint({ static_cast<float>(rect.bottomRight.x), static_cast<float>(rect.topLeft.y) }),
            transform.TransformPoint({ static_cast<float>(rect.topLeft.x),     static_cast<float>(rect.bottomRight.y) }),
            transform.TransformPoint({ static_cast<float>(rect.bottomRight.x), static_cast<float>(rect.bottomRight.y) }),
        };
        float minX = corners[0].x;
        float maxX = corners[0].x;
        float minY = corners[0].y;
        float maxY = corners[0].y;
        for (int i = 1; i < 4; ++i) {
            minX = std::min(minX, corners[i].x);
            maxX = std::max(maxX, corners[i].x);
            minY = std::min(minY, corners[i].y);
            maxY = std::max(maxY, corners[i].y);
        }
        moth::ui::IntRect const screenAabb{
            { static_cast<int>(std::floor(minX)), static_cast<int>(std::floor(minY)) },
            { static_cast<int>(std::ceil(maxX)),  static_cast<int>(std::ceil(maxY)) },
        };

        PushClip(screenAabb);
        m_graphics.DrawGradientRect(static_cast<FloatRect>(rect),
                                    gradient.startColor * tint,
                                    gradient.endColor * tint,
                                    gradient.midpoint,
                                    gradient.angle,
                                    gradient.transitionLength);
        PopClip();
    }

    void MothRenderer::RenderImage(moth::ui::IImage const& image, moth::ui::IntRect const& sourceRect, moth::ui::IntRect const& destRect, moth::ui::ImageScaleType scaleType, float scale) {
        m_graphics.SetBlendMode(m_blendMode.top());
        m_graphics.SetColor(m_drawColor.top());

        auto const* mothImagePtr = dynamic_cast<MothImage const*>(&image);
        if (mothImagePtr == nullptr) {
            return;
        }
        auto const& internalImage = mothImagePtr->GetImage();
        if (!internalImage) {
            return;
        }
        auto const srcRect = sourceRect;

        if (destRect.w() <= 0 || destRect.h() <= 0) {
            return;
        }

        if (auto texture = internalImage.GetTexture()) {
            auto const gfxFilter = ToGraphicsFilter(m_textureFilter.top());
            texture->SetFilter(gfxFilter, gfxFilter);
        }

        if (scaleType == moth::ui::ImageScaleType::Stretch) {
            m_graphics.DrawImage(internalImage, destRect, &srcRect);
        } else if (scaleType == moth::ui::ImageScaleType::Tile) {
            m_graphics.DrawImageTiled(internalImage, destRect, &srcRect, scale);
        }
    }

    void MothRenderer::RenderText(std::string_view text, moth::ui::IFont& font, moth::ui::TextHorizAlignment horizontalAlignment, moth::ui::TextVertAlignment verticalAlignment, moth::ui::IntRect const& destRect) {
        m_graphics.SetBlendMode(m_blendMode.top());
        m_graphics.SetColor(m_drawColor.top());

        auto* fcFontPtr = dynamic_cast<MothFont*>(&font);
        if (fcFontPtr == nullptr) {
            return;
        }
        auto internalFontPtr = fcFontPtr->GetInternalFont();
        if (internalFontPtr == nullptr) {
            return;
        }
        auto& internalFont = *internalFontPtr;
        m_graphics.DrawText(text, internalFont, destRect, horizontalAlignment, verticalAlignment);
    }

    void MothRenderer::SetRendererLogicalSize(moth::ui::IntVec2 const& size) {
        m_graphics.SetLogicalSize(size);
    }
}
