#include "common.h"
#include "moth_ui/nodes/node_flipbook.h"
#include "moth_ui/animation/animation_controller.h"
#include "moth_ui/animation/animation_track.h"
#include "moth_ui/events/event_flipbook.h"
#include "moth_ui/graphics/image_scale_type.h"
#include "moth_ui/layout/layout_entity_flipbook.h"
#include "moth_ui/context.h"

namespace moth_ui {
    NodeFlipbook::NodeFlipbook(Context& context)
        : Node(context) {
    }

    NodeFlipbook::NodeFlipbook(Context& context, std::shared_ptr<LayoutEntityFlipbook> layoutEntity)
        : Node(context, layoutEntity)
        , m_typedLayout(layoutEntity.get()) {
        m_textureFilter = m_typedLayout->m_textureFilter;
        Load(m_typedLayout->m_flipbookPath);

        auto& controller = GetAnimationController();
        controller.ClearDiscreteCallbacks();
        controller.RegisterDiscreteCallback(AnimationTarget::FlipbookClip, [this](std::string_view value) {
            SetClip(value);
        });
        controller.RegisterDiscreteCallback(AnimationTarget::FlipbookPlaying, [this](std::string_view value) {
            SetPlaying(value == "1");
        });
        controller.SetFrameDiscrete(0.0f);
    }

    void NodeFlipbook::UpdateChildBounds() {
    }

    void NodeFlipbook::Load(std::filesystem::path const& path) {
        m_currentFrame = 0;
        m_accumulatedMs = 0.0f;
        m_currentClip.reset();
        m_currentClipName.clear();
        m_flipbook.reset();
        m_playing = false;
        m_pendingStartedEvent = false;
        m_pendingStartedClipName.clear();
        auto* factory = m_context.GetFlipbookFactory();
        if (factory != nullptr) {
            m_flipbook = factory->GetFlipbook(path);
            if (m_flipbook == nullptr) {
                log::warn("NodeFlipbook: failed to load flipbook '{}'", path.string());
            } else if (m_flipbook->GetFrameCount() <= 0) {
                log::error("NodeFlipbook: flipbook '{}' has no frames", path.string());
                m_flipbook.reset();
            }
        }
    }

    void NodeFlipbook::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        m_textureFilter = m_typedLayout->m_textureFilter;
        Load(m_typedLayout->m_flipbookPath);

        auto& controller = GetAnimationController();
        controller.ClearDiscreteCallbacks();
        controller.RegisterDiscreteCallback(AnimationTarget::FlipbookClip, [this](std::string_view value) {
            SetClip(value);
        });
        controller.RegisterDiscreteCallback(AnimationTarget::FlipbookPlaying, [this](std::string_view value) {
            SetPlaying(value == "1");
        });
        controller.SetFrameDiscrete(0.0f);
    }

    void NodeFlipbook::SetClip(std::string_view name) {
        m_accumulatedMs = 0;
        m_currentFrame = 0;
        m_currentClip.reset();
        m_currentClipName.clear();
        m_pendingStartedEvent = false;
        m_pendingStartedClipName.clear();
        if (m_flipbook != nullptr) {
            IFlipbook::ClipDesc clipDesc;
            if (m_flipbook->GetClipDesc(name, clipDesc)) {
                m_currentClip = std::move(clipDesc);
                m_currentClipName = name;
            } else if (!name.empty()) {
                log::warn("NodeFlipbook: clip '{}' not found", name);
            }
        }
        if (!m_currentClip.has_value()) {
            m_playing = false;
        }
    }

    void NodeFlipbook::SetPlaying(bool playing) {
        if (m_flipbook && m_currentClip.has_value()) {
            bool const wasPlaying = m_playing;
            m_playing = playing;
            if (!wasPlaying && m_playing) {
                if (weak_from_this().expired()) {
                    m_pendingStartedEvent = true;
                    m_pendingStartedClipName = m_currentClipName;
                } else {
                    SendEvent(EventFlipbookStarted(SharedFromThis(), m_currentClipName));
                }
            }
        }
    }

    void NodeFlipbook::Update(uint32_t ticks) {
        Node::Update(ticks);

        if (m_pendingStartedEvent) {
            m_pendingStartedEvent = false;
            SendEvent(EventFlipbookStarted(SharedFromThis(), m_pendingStartedClipName));
            m_pendingStartedClipName.clear();
        }

        if (!m_playing || !m_flipbook || !m_currentClip.has_value() || m_currentClip->frames.empty()) {
            return;
        }

        m_accumulatedMs += static_cast<float>(ticks);

        while (m_playing) {
            int const durationMs = std::max(1, m_currentClip->frames[m_currentFrame].durationMs);
            if (m_accumulatedMs < static_cast<float>(durationMs)) {
                break;
            }
            m_accumulatedMs -= static_cast<float>(durationMs);
            ++m_currentFrame;
            int const lastStep = static_cast<int>(m_currentClip->frames.size()) - 1;
            if (m_currentFrame > lastStep) {
                switch (m_currentClip->loop) {
                case IFlipbook::LoopType::Loop:
                    m_currentFrame = 0;
                    break;
                case IFlipbook::LoopType::Reset:
                    m_accumulatedMs = 0;
                    m_currentFrame = 0;
                    m_playing = false;
                    SendEvent(EventFlipbookStopped(SharedFromThis(), m_currentClipName));
                    break;
                case IFlipbook::LoopType::Stop:
                    m_accumulatedMs = 0;
                    m_currentFrame = lastStep;
                    m_playing = false;
                    SendEvent(EventFlipbookStopped(SharedFromThis(), m_currentClipName));
                    break;
                }
            }
        }
    }

    void NodeFlipbook::DrawInternal() {
        if (m_flipbook == nullptr || !m_currentClip.has_value() || m_currentClip->frames.empty()) {
            return;
        }
        int const atlasFrameIndex = m_currentClip->frames[m_currentFrame].frameIndex;
        IFlipbook::FrameDesc frameDesc;
        if (!m_flipbook->GetFrameDesc(atlasFrameIndex, frameDesc)) {
            return;
        }
        auto& renderer = m_context.GetRenderer();
        auto const& image = m_flipbook->GetImage();
        // Scale the frame to fill the node rect, then offset so the scaled pivot lands at the
        // node's layout-pivot point (m_pivot, normalised [0,1], default centre {0.5, 0.5}).
        // This makes the node rect control the display size, consistent with NodeImage.
        int const nodeW = m_screenRect.bottomRight.x - m_screenRect.topLeft.x;
        int const nodeH = m_screenRect.bottomRight.y - m_screenRect.topLeft.y;
        int const frameW = frameDesc.rect.w();
        int const frameH = frameDesc.rect.h();
        float const scaleX = (frameW > 0) ? static_cast<float>(nodeW) / static_cast<float>(frameW) : 1.0f;
        float const scaleY = (frameH > 0) ? static_cast<float>(nodeH) / static_cast<float>(frameH) : 1.0f;
        int const anchorX = static_cast<int>(static_cast<float>(nodeW) * m_pivot.x);
        int const anchorY = static_cast<int>(static_cast<float>(nodeH) * m_pivot.y);
        int const scaledPivotX = static_cast<int>(static_cast<float>(frameDesc.pivot.x) * scaleX);
        int const scaledPivotY = static_cast<int>(static_cast<float>(frameDesc.pivot.y) * scaleY);
        IntRect const destRect{
            { anchorX - scaledPivotX,         anchorY - scaledPivotY },
            { anchorX - scaledPivotX + nodeW,  anchorY - scaledPivotY + nodeH }
        };
        renderer.PushTextureFilter(m_textureFilter);
        renderer.RenderImage(image, frameDesc.rect, destRect, ImageScaleType::Stretch, 1.0f);
        renderer.PopTextureFilter();
    }

    std::shared_ptr<NodeFlipbook> NodeFlipbook::SharedFromThis() {
        return std::static_pointer_cast<NodeFlipbook>(shared_from_this());
    }

    std::shared_ptr<NodeFlipbook> NodeFlipbook::Create(Context& context) {
        return std::shared_ptr<NodeFlipbook>(new NodeFlipbook(context));
    }

    std::shared_ptr<NodeFlipbook> NodeFlipbook::Create(Context& context, std::shared_ptr<LayoutEntityFlipbook> layoutEntity) {
        return std::shared_ptr<NodeFlipbook>(new NodeFlipbook(context, std::move(layoutEntity)));
    }
}
