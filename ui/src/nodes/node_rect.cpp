#include "common.h"
#include "moth_ui/nodes/node_rect.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/context.h"

namespace moth_ui {
    NodeRect::NodeRect(Context& context)
        : Node(context) {
    }

    NodeRect::NodeRect(Context& context, std::shared_ptr<LayoutEntityRect> layoutEntity)
        : Node(context, layoutEntity) {
        auto const layoutEntityPtr = std::static_pointer_cast<LayoutEntityRect>(m_layout);
        m_filled = layoutEntityPtr->m_filled;
    }

    void NodeRect::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        auto const layoutEntityPtr = std::static_pointer_cast<LayoutEntityRect>(m_layout);
        m_filled = layoutEntityPtr->m_filled;
    }

    void NodeRect::DrawInternal() {
        IntRect const localRect{ { 0, 0 }, m_screenRect.bottomRight - m_screenRect.topLeft };
        if (m_filled) {
            m_context.GetRenderer().RenderFilledRect(localRect);
        } else {
            m_context.GetRenderer().RenderRect(localRect);
        }
    }
}
