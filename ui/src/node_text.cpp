#include "common.h"
#include "moth_ui/node_text.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/context.h"

namespace moth_ui {
    NodeText::NodeText() {
    }

    NodeText::NodeText(std::shared_ptr<LayoutEntityText> layoutEntity)
        : Node(layoutEntity) {
        ReloadEntityPrivate();
    }

    NodeText::~NodeText() {
    }

    void NodeText::Load(char const* fontName, int size) {
        if (fontName == nullptr || *fontName == 0) {
            m_font = Context::GetCurrentContext()->GetFontFactory().GetDefaultFont(size);
        } else {
            m_font = Context::GetCurrentContext()->GetFontFactory().GetFont(fontName, size);
        }
    }

    void NodeText::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        ReloadEntityPrivate();
    }

    void NodeText::DrawInternal() {
        if (m_font) {
            //auto& renderer = Context::GetCurrentContext()->GetRenderer();
            //if (m_dropShadow) {
            //    // pop the color so the dropshadow color isnt affected by the node color unlike the text
            //    renderer.PopColor();
            //    renderer.PushColor(m_dropShadowColor);
            //    IntRect dropRect = m_screenRect;
            //    dropRect.topLeft += m_dropShadowOffset;
            //    dropRect.bottomRight += m_dropShadowOffset;
            //    renderer.RenderText(m_text, *m_font, m_horizontalAlignment, m_verticalAlignment, dropRect);
            //    renderer.PopColor();
            //    renderer.PushColor(m_color);
            //}
            //renderer.RenderText(m_text, *m_font, m_horizontalAlignment, m_verticalAlignment, m_screenRect);
        }
    }

    void NodeText::ReloadEntityPrivate() {
        //auto const layoutEntity = std::static_pointer_cast<LayoutEntityText>(m_layout);
        //m_text = layoutEntity->m_text;
        //m_horizontalAlignment = layoutEntity->m_horizontalAlignment;
        //m_verticalAlignment = layoutEntity->m_verticalAlignment;
        //m_dropShadow = layoutEntity->m_dropShadow;
        //m_dropShadowOffset = layoutEntity->m_dropShadowOffset;
        //m_dropShadowColor = layoutEntity->m_dropShadowColor;
        //Load(layoutEntity->m_fontName.c_str(), layoutEntity->m_fontSize);
    }
}
