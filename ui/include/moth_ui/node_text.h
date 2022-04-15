#pragma once

#include "node.h"
#include "text_alignment.h"

namespace moth_ui {
    class NodeText : public Node {
    public:
        NodeText();
        NodeText(std::shared_ptr<LayoutEntityText> layoutEntity);
        virtual ~NodeText();

        void Load(char const* fontName, int size);

        void ReloadEntity() override;
        IFont* GetFont() const {
            return m_font.get();
        }

        void SetText(char const* text) { m_text = text; }

        void DebugDraw();

    protected:
        std::unique_ptr<IFont> m_font;
        std::string m_text;
        TextHorizAlignment m_horizontalAlignment;
        TextVertAlignment m_verticalAlignment;

        void DrawInternal() override;
    };
}
