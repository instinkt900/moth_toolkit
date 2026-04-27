#include "common.h"
#include "moth_ui/nodes/node_rect.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/context.h"

namespace moth_ui {
    NodeRect::NodeRect(Context& context)
        : Node(context) {
    }

    NodeRect::NodeRect(Context& context, std::shared_ptr<LayoutEntityRect> layoutEntity)
        : Node(context, layoutEntity)
        , m_typedLayout(layoutEntity.get()) {
        m_filled = m_typedLayout->m_filled;
    }

    void NodeRect::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        m_filled = m_typedLayout->m_filled;
    }

    void NodeRect::DrawInternal() {
        IntRect const localRect{ { 0, 0 }, m_screenRect.bottomRight - m_screenRect.topLeft };
        if (m_filled) {
            m_context.GetRenderer().RenderFilledRect(localRect);
        } else {
            m_context.GetRenderer().RenderRect(localRect);
        }
    }

    std::shared_ptr<NodeRect> NodeRect::Create(Context& context) {
        return std::shared_ptr<NodeRect>(new NodeRect(context));
    }

    std::shared_ptr<NodeRect> NodeRect::Create(Context& context, std::shared_ptr<LayoutEntityRect> layoutEntity) {
        return std::shared_ptr<NodeRect>(new NodeRect(context, std::move(layoutEntity)));
    }
}
