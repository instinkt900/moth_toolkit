#pragma once

#include "layout_entity.h"
#include "moth_ui/text_alignment.h"

namespace moth_ui {
    class LayoutEntityText : public LayoutEntity {
    public:
        LayoutEntityText(LayoutRect const& initialBounds);
        LayoutEntityText(nlohmann::json const& json, LayoutEntityGroup* parent);

        LayoutEntityType GetType() const override { return LayoutEntityType::Text; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize() const override;
        void Deserialize(nlohmann::json const& json) override;

        std::string m_fontPath;
        int m_fontSize = 16;
        std::string m_text = "Text Element";
        TextAlignment m_alignment = TextAlignment::Left;
    };
}
