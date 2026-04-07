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
        : Node(context, layoutEntity) {
        ReloadEntityPrivate();
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
                GetLogger().Warning("NodeFlipbook: failed to load flipbook '{}'", path.string());
            } else if (m_flipbook->GetFrameCount() <= 0) {
                GetLogger().Error("NodeFlipbook: flipbook '{}' has no frames", path.string());
                m_flipbook.reset();
            }
        }
    }

    void NodeFlipbook::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        ReloadEntityPrivate();
    }

    void NodeFlipbook::ReloadEntityPrivate() {
        auto const layoutEntity = std::static_pointer_cast<LayoutEntityFlipbook>(m_layout);
        Load(layoutEntity->m_flipbookPath);

        auto& controller = GetAnimationController();
        controller.ClearDiscreteCallbacks();
        controller.RegisterDiscreteCallback(AnimationTrack::Target::FlipbookClip, [this](std::string_view value) {
            SetClip(value);
        });
        controller.RegisterDiscreteCallback(AnimationTrack::Target::FlipbookPlaying, [this](std::string_view value) {
            SetPlaying(value == "1");
        });
        // Apply initial state from discrete tracks at frame 0.
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
                GetLogger().Warning("NodeFlipbook: clip '{}' not found", name);
            }
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
                    SendEventUp(EventFlipbookStarted(SharedFromThis(), m_currentClipName));
                }
            }
        }
    }

    void NodeFlipbook::Update(uint32_t ticks) {
        Node::Update(ticks);

        if (m_pendingStartedEvent) {
            m_pendingStartedEvent = false;
            SendEventUp(EventFlipbookStarted(SharedFromThis(), m_pendingStartedClipName));
            m_pendingStartedClipName.clear();
        }

        if (!m_playing || !m_flipbook || !m_currentClip.has_value() || m_currentClip->frames.empty()) {
            return;
        }

        m_accumulatedMs += static_cast<float>(ticks);

        while (m_playing) {
            int const durationMs = m_currentClip->frames[m_currentFrame].durationMs;
            if (durationMs <= 0 || m_accumulatedMs < static_cast<float>(durationMs)) {
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
                    SendEventUp(EventFlipbookStopped(SharedFromThis(), m_currentClipName));
                    break;
                case IFlipbook::LoopType::Stop:
                    m_accumulatedMs = 0;
                    m_currentFrame = lastStep;
                    m_playing = false;
                    SendEventUp(EventFlipbookStopped(SharedFromThis(), m_currentClipName));
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
        IntRect const localRect{ { 0, 0 }, m_screenRect.bottomRight - m_screenRect.topLeft };
        renderer.RenderImage(image, frameDesc.rect, localRect, ImageScaleType::Stretch, 1.0f);
    }

    std::shared_ptr<NodeFlipbook> NodeFlipbook::SharedFromThis() {
        return std::static_pointer_cast<NodeFlipbook>(shared_from_this());
    }
}
