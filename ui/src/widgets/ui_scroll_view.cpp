#include "common.h"
#include "moth_ui/widgets/ui_scroll_view.h"

#include "moth_ui/context.h"
#include "moth_ui/events/event_mouse.h"
#include "moth_ui/graphics/irenderer.h"

#include <algorithm>
#include <optional>

namespace moth_ui {
    namespace {
        // Returns the pointer position for the positional mouse events, or
        // nullopt for any other event type.
        std::optional<IntVec2> PositionOf(Event const& event) {
            if (auto const* e = event_cast<EventMouseDown>(event)) {
                return e->GetPosition();
            }
            if (auto const* e = event_cast<EventMouseUp>(event)) {
                return e->GetPosition();
            }
            if (auto const* e = event_cast<EventMouseMove>(event)) {
                return e->GetPosition();
            }
            return std::nullopt;
        }
    }

    UIScrollView::UIScrollView(Context& context, std::shared_ptr<LayoutEntityGroup> entity)
        : Widget<UIScrollView>(context, std::move(entity)) {
    }

    void UIScrollView::SetContent(std::shared_ptr<Node> content) {
        if (m_content) {
            RemoveChild(m_content);
        }
        m_content = std::move(content);
        m_scrollY = 0.0f;
        if (m_content) {
            AddChild(m_content);
            auto const& offset = m_content->GetLayoutRect().offset;
            m_baseOffsetTop = offset.topLeft.y;
            m_baseOffsetBottom = offset.bottomRight.y;
            m_content->RecalculateBounds();
        }
    }

    bool UIScrollView::Broadcast(Event const& event) {
        // Scroll when the wheel turns over this viewport, and consume the event
        // so it doesn't reach another scroller or the layer behind us. The wheel
        // event carries the current cursor position, so this works even on the
        // first scroll after the view appears under a stationary pointer, with
        // no prior move event to seed it.
        if (auto const* wheel = event_cast<EventMouseWheel>(event)) {
            if (IsInBounds(wheel->GetPosition())) {
                ApplyScroll(static_cast<float>(-wheel->GetDelta().y) * m_scrollStep);
                return true;
            }
            return false;
        }

        // Drop pointer events outside the viewport before they descend into the
        // content, so rows scrolled out of sight can't be hovered or clicked.
        if (auto const position = PositionOf(event)) {
            if (!IsInBounds(*position)) {
                return false;
            }
        }

        return Group::Broadcast(event);
    }

    void UIScrollView::DrawInternal() {
        m_context.GetRenderer().PushClip(GetScreenRect());
        Group::DrawInternal();
        m_context.GetRenderer().PopClip();
    }

    void UIScrollView::ApplyScroll(float deltaY) {
        if (!m_content) {
            return;
        }
        m_scrollY = std::clamp(m_scrollY + deltaY, 0.0f, MaxScroll());
        auto& offset = m_content->GetLayoutRect().offset;
        offset.topLeft.y = m_baseOffsetTop - m_scrollY;
        offset.bottomRight.y = m_baseOffsetBottom - m_scrollY;
        m_content->RecalculateBounds();
    }

    float UIScrollView::MaxScroll() const {
        if (!m_content) {
            return 0.0f;
        }
        float const contentH = static_cast<float>(m_content->GetScreenRect().h());
        float const viewportH = static_cast<float>(GetScreenRect().h());
        return std::max(0.0f, contentH - viewportH);
    }
}
