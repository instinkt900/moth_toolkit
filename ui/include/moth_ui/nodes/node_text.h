#pragma once

#include "moth_ui/graphics/text_alignment.h"
#include "moth_ui/nodes/node.h"

#include <memory>
#include <string>
#include <string_view>

namespace moth_ui {
    /**
     * @brief A Node that renders a text string using a named font.
     *
     * Supports horizontal and vertical alignment, and optional drop-shadow.
     */
    class NodeText : public Node {
    public:
        /**
         * @brief Constructs a NodeText with no layout entity.
         * @param context Active rendering context.
         */
        NodeText(Context& context);

        /**
         * @brief Constructs a NodeText from a serialised layout entity.
         * @param context      Active rendering context.
         * @param layoutEntity Deserialised text description.
         */
        NodeText(Context& context, std::shared_ptr<LayoutEntityText> layoutEntity);
        NodeText(NodeText const& other) = delete;
        NodeText(NodeText&& other) = default;
        NodeText& operator=(NodeText const&) = delete;
        NodeText& operator=(NodeText&&) = delete;
        ~NodeText() override = default;

        /**
         * @brief Loads a font by name and point size from the font factory.
         * @param fontName Name of the font registered with the font factory.
         * @param size     Point size to load.
         */
        void Load(std::string_view fontName, int size);

        /**
         * @brief Returns the currently loaded font, or @c nullptr.
         */
        IFont* GetFont() const {
            return m_font.get();
        }

        /**
         * @brief Sets the text string to display.
         * @param text UTF-8 string.
         */
        void SetText(std::string_view text) { m_text = text; }

        /// @brief Returns the text string currently displayed.
        std::string const& GetText() const { return m_text; }

        /// @brief Returns the horizontal text alignment.
        TextHorizAlignment GetHorizontalAlignment() const { return m_horizontalAlignment; }

        /// @brief Returns the vertical text alignment.
        TextVertAlignment GetVerticalAlignment() const { return m_verticalAlignment; }

        /// @brief Returns @c true if a drop-shadow is rendered.
        bool IsDropShadow() const { return m_dropShadow; }

        /// @brief Returns the drop-shadow pixel offset.
        IntVec2 const& GetDropShadowOffset() const { return m_dropShadowOffset; }

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
