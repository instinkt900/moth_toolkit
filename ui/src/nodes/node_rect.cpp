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
        ReloadEntityPrivate();
    }

    void NodeRect::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        ReloadEntityPrivate();
    }

    void NodeRect::DrawInternal() {
        if (m_filled) {
            m_context.GetRenderer().RenderFilledRect(m_screenRect);
        } else {
            m_context.GetRenderer().RenderRect(m_screenRect);
        }
    }

    void NodeRect::ReloadEntityPrivate() {
        auto layoutEntity = std::static_pointer_cast<LayoutEntityRect>(m_layout);
        m_filled = layoutEntity->m_filled;
    }
}
