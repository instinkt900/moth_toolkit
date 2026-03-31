#include "common.h"
#include "moth_ui/nodes/node_flipbook.h"
#include "moth_ui/events/event_flipbook.h"
#include "moth_ui/graphics/image_scale_type.h"
#include "moth_ui/layout/layout_entity.h"
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
        auto* factory = m_context.GetFlipbookFactory();
        if (factory != nullptr) {
            m_flipbook = factory->GetFlipbook(path);
        }
    }

    void NodeFlipbook::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        ReloadEntityPrivate();
    }

    void NodeFlipbook::ReloadEntityPrivate() {
        auto const layoutEntity = std::static_pointer_cast<LayoutEntityFlipbook>(m_layout);
        m_currentFrame = 0;
        m_accumulatedMs = 0.0f;
        m_playing = false;
        Load(layoutEntity->m_flipbookPath);
        if (m_flipbook) {
            m_playing = true;
            SendEventUp(EventFlipbookStarted(this));
        }
    }

    void NodeFlipbook::Update(uint32_t ticks) {
        Node::Update(ticks);
        if (!m_playing || !m_flipbook || m_flipbook->GetFps() <= 0) {
            return;
        }

        float const frameDurationMs = 1000.0f / static_cast<float>(m_flipbook->GetFps());
        m_accumulatedMs += static_cast<float>(ticks);

        while (m_accumulatedMs >= frameDurationMs) {
            m_accumulatedMs -= frameDurationMs;
            ++m_currentFrame;
            if (m_currentFrame >= m_flipbook->GetMaxFrames()) {
                if (m_flipbook->GetLoop()) {
                    m_currentFrame = 0;
                } else {
                    m_currentFrame = m_flipbook->GetMaxFrames() - 1;
                    m_playing = false;
                    SendEventUp(EventFlipbookStopped(this));
                    break;
                }
            }
        }
    }

    void NodeFlipbook::DrawInternal() {
        if (m_flipbook == nullptr) {
            return;
        }
        auto& renderer = m_context.GetRenderer();
        int const col = m_currentFrame % m_flipbook->GetFrameCols();
        int const row = m_currentFrame / m_flipbook->GetFrameCols();
        IntRect const srcRect = MakeRect(col * m_flipbook->GetFrameWidth(),
                                         row * m_flipbook->GetFrameHeight(),
                                         m_flipbook->GetFrameWidth(),
                                         m_flipbook->GetFrameHeight());
        auto& image = m_flipbook->GetImage();
        IntRect const localRect{ { 0, 0 }, m_screenRect.bottomRight - m_screenRect.topLeft };
        renderer.RenderImage(image, srcRect, localRect, ImageScaleType::Stretch, 1.0f);
    }
}
