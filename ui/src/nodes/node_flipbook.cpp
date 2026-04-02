#include "common.h"
#include "moth_ui/nodes/node_flipbook.h"
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
        m_sheetDesc.reset();
        m_flipbook.reset();
        m_playing = false;
        auto* factory = m_context.GetFlipbookFactory();
        if (factory != nullptr) {
            m_flipbook = factory->GetFlipbook(path);
            if (m_flipbook == nullptr) {
                GetLogger().Warning("NodeFlipbook: failed to load flipbook '{}'", path.string());
            } else {
                m_sheetDesc.emplace();
                m_flipbook->GetSheetDesc(*m_sheetDesc);
                if (m_sheetDesc->SheetCells.x <= 0 || m_sheetDesc->SheetCells.y <= 0 || m_sheetDesc->FrameDimensions.x <= 0 || m_sheetDesc->FrameDimensions.y <= 0) {
                    GetLogger().Error("NodeFlipbook: invalid sheet descriptor in '{}' (cells={}x{} frame={}x{})",
                        path.string(),
                        m_sheetDesc->SheetCells.x, m_sheetDesc->SheetCells.y,
                        m_sheetDesc->FrameDimensions.x, m_sheetDesc->FrameDimensions.y);
                    m_sheetDesc.reset();
                    m_flipbook.reset();
                    return;
                }
                SetClip(m_initialClipName);
                if (m_autoplay) {
                    SetPlaying(true);
                }
            }
        }
    }

    void NodeFlipbook::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        ReloadEntityPrivate();
    }

    void NodeFlipbook::ReloadEntityPrivate() {
        auto const layoutEntity = std::static_pointer_cast<LayoutEntityFlipbook>(m_layout);
        m_initialClipName = layoutEntity->m_clipName;
        m_autoplay = layoutEntity->m_autoplay;
        Load(layoutEntity->m_flipbookPath);
    }

    void NodeFlipbook::SetClip(std::string_view name) {
        m_accumulatedMs = 0;
        m_currentFrame = 0;
        m_currentClip.reset();
        m_currentClipName.clear();
        if (m_flipbook != nullptr) {
            IFlipbook::ClipDesc clipDesc;
            if (m_flipbook->GetClipDesc(name, clipDesc)) {
                m_currentClip = clipDesc;
                m_currentClipName = name;
                m_currentFrame = m_currentClip->Start;
            } else if (!name.empty()) {
                GetLogger().Warning("NodeFlipbook: clip '{}' not found", name);
            }
        }
    }

    void NodeFlipbook::SetPlaying(bool playing) {
        if (m_flipbook && m_currentClip.has_value()) {
            bool wasPlaying = m_playing;
            m_playing = playing;
            if (!wasPlaying && m_playing) {
                SendEventUp(EventFlipbookStarted(SharedFromThis(), m_currentClipName));
            }
        }
    }

    void NodeFlipbook::Update(uint32_t ticks) {
        Node::Update(ticks);

        if (!m_playing || !m_flipbook || !m_currentClip.has_value()) {
            return;
        }

        if (m_currentClip->FPS <= 0) {
            return;
        }
        float const frameDurationMs = 1000.0f / static_cast<float>(m_currentClip->FPS);
        m_accumulatedMs += static_cast<float>(ticks);

        while (m_playing && m_accumulatedMs >= frameDurationMs) {
            m_accumulatedMs -= frameDurationMs;
            ++m_currentFrame;
            if (m_currentFrame > m_currentClip->End) {
                switch (m_currentClip->Loop) {
                case IFlipbook::LoopType::Loop:
                    m_currentFrame = m_currentClip->Start;
                    break;
                case IFlipbook::LoopType::Reset:
                    m_accumulatedMs = 0;
                    m_currentFrame = m_currentClip->Start;
                    m_playing = false;
                    SendEventUp(EventFlipbookStopped(SharedFromThis(), m_currentClipName));
                    break;
                case IFlipbook::LoopType::Stop:
                    m_accumulatedMs = 0;
                    m_currentFrame = m_currentClip->End;
                    m_playing = false;
                    SendEventUp(EventFlipbookStopped(SharedFromThis(), m_currentClipName));
                    break;
                }
            }
        }
    }

    void NodeFlipbook::DrawInternal() {
        if (m_flipbook == nullptr || !m_sheetDesc.has_value()) {
            return;
        }
        auto& renderer = m_context.GetRenderer();
        int const col = m_currentFrame % m_sheetDesc->SheetCells.x;
        int const row = m_currentFrame / m_sheetDesc->SheetCells.x;
        IntRect const srcRect = MakeRect(col * m_sheetDesc->FrameDimensions.x,
                                         row * m_sheetDesc->FrameDimensions.y,
                                         m_sheetDesc->FrameDimensions.x,
                                         m_sheetDesc->FrameDimensions.y);
        auto const& image = m_flipbook->GetImage();
        IntRect const localRect{ { 0, 0 }, m_screenRect.bottomRight - m_screenRect.topLeft };
        renderer.RenderImage(image, srcRect, localRect, ImageScaleType::Stretch, 1.0f);
    }

    std::shared_ptr<NodeFlipbook> NodeFlipbook::SharedFromThis() {
        return std::static_pointer_cast<NodeFlipbook>(shared_from_this());
    }
}
