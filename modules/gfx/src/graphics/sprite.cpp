#include "common.h"
#include "moth/graphics/graphics/sprite.h"
#include "moth/graphics/graphics/igraphics.h"

namespace moth::gfx {
    Sprite::Sprite(std::shared_ptr<SpriteSheet> spriteSheet)
        : m_spriteSheet(std::move(spriteSheet)) {
    }

    void Sprite::SetClip(std::string_view name) {
        m_accumulatedMs = 0.0f;
        m_currentFrame = 0;
        m_currentClip.reset();
        m_currentClipName.clear();

        if (name.empty()) {
            m_playing = false;
            return;
        }

        auto clipDesc = m_spriteSheet->GetClipDesc(name);
        if (clipDesc) {
            m_currentClip = std::move(*clipDesc);
            m_currentClipName = name;
            // A clip selected while already playing begins advancing immediately.
            if (m_playing && OnClipStarted) {
                OnClipStarted(m_currentClipName);
            }
        } else {
            moth::core::log::warn("Sprite::SetClip: clip '{}' not found", name);
            m_playing = false;
        }
    }

    void Sprite::SetFrame(int frame) {
        m_playing = false;
        m_currentClip.reset();
        m_currentClipName.clear();
        m_accumulatedMs = 0.0f;
        m_currentFrame = std::clamp(frame, 0, m_spriteSheet->GetFrameCount() - 1);
    }

    void Sprite::SetPlaying(bool playing) {
        if (m_currentClip.has_value()) {
            bool const wasPlaying = m_playing;
            m_playing = playing;
            if (!wasPlaying && m_playing && OnClipStarted) {
                OnClipStarted(m_currentClipName);
            }
        }
    }

    void Sprite::SetSpeed(float speed) {
        if (speed <= 0.0f) {
            moth::core::log::warn("Sprite::SetSpeed: speed must be > 0 (got {}); resetting to 1.0", speed);
            m_speed = 1.0f;
            return;
        }
        m_speed = speed;
    }

    void Sprite::Update(uint32_t ticks) {
        if (!m_playing || !m_currentClip.has_value() || m_currentClip->frames.empty()) {
            return;
        }

        m_accumulatedMs += static_cast<float>(ticks) * m_speed;

        bool stopped = false;
        while (m_playing) {
            int const durationMs = m_currentClip->frames[static_cast<size_t>(m_currentFrame)].durationMs;
            if (durationMs <= 0 || m_accumulatedMs < static_cast<float>(durationMs)) {
                break;
            }
            m_accumulatedMs -= static_cast<float>(durationMs);
            ++m_currentFrame;
            int const lastStep = static_cast<int>(m_currentClip->frames.size()) - 1;
            if (m_currentFrame > lastStep) {
                switch (m_currentClip->loop) {
                case SpriteSheet::LoopType::Loop:
                    m_currentFrame = 0;
                    if (OnClipLooped) {
                        OnClipLooped(m_currentClipName);
                    }
                    break;
                case SpriteSheet::LoopType::Reset:
                    m_accumulatedMs = 0.0f;
                    m_currentFrame = 0;
                    m_playing = false;
                    stopped = true;
                    break;
                case SpriteSheet::LoopType::Stop:
                    m_accumulatedMs = 0.0f;
                    m_currentFrame = lastStep;
                    m_playing = false;
                    stopped = true;
                    break;
                }
            }
        }

        if (stopped && OnClipStopped) {
            OnClipStopped(m_currentClipName);
        }
    }

    int Sprite::GetCurrentFrame() const {
        if (m_currentClip.has_value() && !m_currentClip->frames.empty()) {
            return m_currentClip->frames[static_cast<size_t>(m_currentFrame)].frameIndex;
        }
        return m_currentFrame;
    }

    IntRect Sprite::GetCurrentFrameRect() const {
        auto entry = m_spriteSheet->GetFrameDesc(GetCurrentFrame());
        if (entry) {
            return entry->rect;
        }
        return {};
    }

    IntVec2 Sprite::GetCurrentFramePivot() const {
        auto entry = m_spriteSheet->GetFrameDesc(GetCurrentFrame());
        if (entry) {
            return entry->pivot;
        }
        return {};
    }

    int Sprite::GetWidth() const {
        auto const rect = GetCurrentFrameRect();
        return rect.bottomRight.x - rect.topLeft.x;
    }

    int Sprite::GetHeight() const {
        auto const rect = GetCurrentFrameRect();
        return rect.bottomRight.y - rect.topLeft.y;
    }

    Image const& Sprite::GetImage() const {
        return m_spriteSheet->GetImage();
    }

    namespace {
        // Builds an Image limited to the sprite's current frame. The Transform2D
        // overload of DrawImage samples Image::GetSourceRect(), so a sub-region
        // image is how a single atlas frame is selected while still honouring flip.
        Image MakeFrameImage(Sprite const& sprite) {
            auto const& image = sprite.GetImage();
            return Image{ image.GetTexture(), sprite.GetCurrentFrameRect() };
        }

        FloatVec2 NormalizedPivot(IntRect const& frameRect, IntVec2 const& pixelPivot) {
            int const w = frameRect.w();
            int const h = frameRect.h();
            return FloatVec2{
                (w > 0) ? static_cast<float>(pixelPivot.x) / static_cast<float>(w) : 0.0f,
                (h > 0) ? static_cast<float>(pixelPivot.y) / static_cast<float>(h) : 0.0f,
            };
        }
    }

    void DrawSprite(IGraphics& graphics, Sprite const& sprite, IntRect const& destRect) {
        auto const& image = sprite.GetImage();
        if (!image) {
            return;
        }
        auto const frameRect = sprite.GetCurrentFrameRect();
        int const w = frameRect.w();
        int const h = frameRect.h();
        Transform2D transform;
        transform.position = FloatVec2{
            static_cast<float>(destRect.x()) + static_cast<float>(destRect.w()) * 0.5f,
            static_cast<float>(destRect.y()) + static_cast<float>(destRect.h()) * 0.5f,
        };
        if (w > 0 && h > 0) {
            transform.scale = FloatVec2{
                static_cast<float>(destRect.w()) / static_cast<float>(w),
                static_cast<float>(destRect.h()) / static_cast<float>(h),
            };
        }
        graphics.DrawImage(MakeFrameImage(sprite), transform, { 0.5f, 0.5f }, sprite.GetFlipX(), false);
    }

    void DrawSprite(IGraphics& graphics, Sprite const& sprite, IntVec2 const& pos, FloatVec2 const& pivot) {
        auto const& image = sprite.GetImage();
        if (!image) {
            return;
        }
        Transform2D transform;
        transform.position = FloatVec2{ static_cast<float>(pos.x), static_cast<float>(pos.y) };
        graphics.DrawImage(MakeFrameImage(sprite), transform, pivot, sprite.GetFlipX(), false);
    }

    void DrawSprite(IGraphics& graphics, Sprite const& sprite, IntVec2 const& pos) {
        auto const& image = sprite.GetImage();
        if (!image) {
            return;
        }
        auto const frameRect = sprite.GetCurrentFrameRect();
        auto const pivot = NormalizedPivot(frameRect, sprite.GetCurrentFramePivot());
        Transform2D transform;
        transform.position = FloatVec2{ static_cast<float>(pos.x), static_cast<float>(pos.y) };
        graphics.DrawImage(MakeFrameImage(sprite), transform, pivot, sprite.GetFlipX(), false);
    }
}
