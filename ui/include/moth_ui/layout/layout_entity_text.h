#pragma once

#include "moth_ui/graphics/text_alignment.h"
#include "moth_ui/layout/layout_entity.h"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <memory>

namespace moth_ui {
    /**
     * @brief Layout entity that describes a text node.
     *
     * Stores the font name, point size, text content, alignment settings, and
     * optional drop-shadow parameters used when instantiating a NodeText.
     */
    class LayoutEntityText : public LayoutEntity {
    public:
        /**
         * @brief Constructs a text entity with an explicit initial bounds.
         * @param initialBounds Starting layout rect.
         */
        explicit LayoutEntityText(LayoutRect const& initialBounds);

        /**
         * @brief Constructs a text entity as a child of the given parent.
         * @param parent Owning group.
         */
        explicit LayoutEntityText(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        /// @brief Returns @c LayoutEntityType::Text.
        LayoutEntityType GetType() const override { return LayoutEntityType::Text; }

        std::shared_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        nlohmann::json SerializeOverrides() const override;
        void DeserializeOverrides(nlohmann::json const& overridesJson) override;

        static int constexpr kDefaultFontSize = 16; ///< Default font point size.

        std::string m_fontName;                                                     ///< Registered font name used to load the font.
        int m_fontSize = kDefaultFontSize;                                           ///< Font point size.
        std::string m_text = "Text Element";                                        ///< Text content to display.
        TextHorizAlignment m_horizontalAlignment = TextHorizAlignment::Left;        ///< Horizontal text alignment.
        TextVertAlignment m_verticalAlignment = TextVertAlignment::Top;             ///< Vertical text alignment.
        bool m_dropShadow = false;                                                  ///< Whether a drop-shadow is rendered.
        IntVec2 m_dropShadowOffset{ 0, 0 };                                         ///< Pixel offset of the drop-shadow from the text.
        Color m_dropShadowColor = BasicColors::Black;                               ///< Colour of the drop-shadow.

        LayoutEntityText(LayoutEntityText const& other) = default;
        LayoutEntityText(LayoutEntityText&& other) = default;
        LayoutEntityText& operator=(LayoutEntityText const&) = delete;
        LayoutEntityText& operator=(LayoutEntityText&&) = default;
        ~LayoutEntityText() override = default;
    };
}
