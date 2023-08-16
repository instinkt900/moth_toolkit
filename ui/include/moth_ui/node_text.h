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

        IFont* GetFont() const {
            return m_font.get();
        }

        void SetText(std::string const& text) { m_text = text; }

    protected:
        std::shared_ptr<IFont> m_font;
        std::string m_text;
        TextHorizAlignment m_horizontalAlignment;
        TextVertAlignment m_verticalAlignment;
        bool m_dropShadow;
        IntVec2 m_dropShadowOffset;
        Color m_dropShadowColor;

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        void ReloadEntityPrivate();
    };
}
