#include "common.h"
#include "moth/ui/nodes/node_gradient.h"
#include "moth/ui/layout/layout_entity_gradient.h"
#include "moth/ui/context.h"
#include "moth/ui/graphics/irenderer.h"

namespace moth::ui {
    NodeGradient::NodeGradient(Context& context)
        : Node(context) {
    }

    NodeGradient::NodeGradient(Context& context, std::shared_ptr<LayoutEntityGradient> layoutEntity)
        : Node(context, layoutEntity)
        , m_typedLayout(layoutEntity.get()) {
        m_gradient = m_typedLayout->GetGradientAtFrame(0.0f);
    }

    void NodeGradient::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        if (m_typedLayout != nullptr) {
            m_gradient = m_typedLayout->GetGradientAtFrame(0.0f);
        }
    }

    void NodeGradient::DrawInternal() {
        IntRect const localRect{ { 0, 0 }, m_screenRect.bottomRight - m_screenRect.topLeft };
        m_context.GetRenderer().RenderGradientRect(localRect, m_gradient);
    }

    std::shared_ptr<NodeGradient> NodeGradient::Create(Context& context) {
        return std::shared_ptr<NodeGradient>(new NodeGradient(context));
    }

    std::shared_ptr<NodeGradient> NodeGradient::Create(Context& context, std::shared_ptr<LayoutEntityGradient> layoutEntity) {
        return std::shared_ptr<NodeGradient>(new NodeGradient(context, std::move(layoutEntity)));
    }
}
