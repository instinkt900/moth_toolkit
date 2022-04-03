#pragma once

#include "node.h"
#include "layout/layout_types.h"
#include "text_alignment.h"

namespace moth_ui {
    class NodeText : public Node {
    public:
        NodeText();
        NodeText(std::shared_ptr<LayoutEntityText> layoutEntity);
        virtual ~NodeText();

        void Load(char const* path, int size);

        void ReloadEntity() override;
        IFont* GetFont() const {
            return m_font.get();
        }

        void DebugDraw();

    protected:
        std::unique_ptr<IFont> m_font;
        std::string m_text;
        TextAlignment m_alignment;

        void DrawInternal() override;
    };
}
