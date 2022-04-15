#include "common.h"
#include "moth_ui/node_rect.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/context.h"

namespace moth_ui {
    NodeRect::NodeRect() {
    }

    NodeRect::NodeRect(std::shared_ptr<LayoutEntityRect> layoutEntity)
        : Node(layoutEntity)
        , m_filled(layoutEntity->m_filled) {
    }

    NodeRect::~NodeRect() {
    }

    void NodeRect::ReloadEntity() {
        Node::ReloadEntity();
        auto layoutEntity = std::static_pointer_cast<LayoutEntityRect>(m_layout);
        m_filled = layoutEntity->m_filled;
    }

    void NodeRect::DrawInternal() {
        if (m_filled) {
            Context::GetCurrentContext().GetRenderer().RenderFilledRect(m_screenRect);
        } else {
            Context::GetCurrentContext().GetRenderer().RenderRect(m_screenRect);
        }
    }
}
