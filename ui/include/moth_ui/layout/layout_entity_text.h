#pragma once

#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/text_alignment.h"

#include <nlohmann/json_fwd.hpp>

#include <string>
#include <memory>

namespace moth_ui {
    class LayoutEntityText : public LayoutEntity {
    public:
        explicit LayoutEntityText(LayoutRect const& initialBounds);
        explicit LayoutEntityText(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        LayoutEntityType GetType() const override { return LayoutEntityType::Text; }

        std::unique_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        virtual nlohmann::json SerializeOverrides() const override;
        virtual void DeserializeOverrides(nlohmann::json const& overridesJson) override;

        std::string m_fontName;
        int m_fontSize = 16;
        std::string m_text = "Text Element";
        TextHorizAlignment m_horizontalAlignment = TextHorizAlignment::Left;
        TextVertAlignment m_verticalAlignment = TextVertAlignment::Top;
        bool m_dropShadow = false;
        IntVec2 m_dropShadowOffset{ 0, 0 };
        Color m_dropShadowColor = BasicColors::Black;
    };
}
