#pragma once

#include "moth_ui/graphics/text_alignment.h"
#include "moth_ui/nodes/node.h"

#include <memory>
#include <string>

namespace moth_ui {
    class NodeText : public Node {
    public:
        NodeText(Context& context);
        NodeText(Context& context, std::shared_ptr<LayoutEntityText> layoutEntity);
        NodeText(NodeText const& other) = delete;
        NodeText(NodeText&& other) = default;
        NodeText& operator=(NodeText const&) = delete;
        NodeText& operator=(NodeText&&) = delete;
        ~NodeText() override = default;

        void Load(char const* fontName, int size);

        IFont* GetFont() const {
            return m_font.get();
        }

        void SetText(std::string const& text) { m_text = text; }
        std::string const& GetText() const { return m_text; }

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
