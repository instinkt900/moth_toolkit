#include "common.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/node_text.h"

namespace moth_ui {
    LayoutEntityText::LayoutEntityText(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityText::LayoutEntityText(nlohmann::json const& json, LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
        Deserialize(json);
    }

    std::unique_ptr<Node> LayoutEntityText::Instantiate() {
        return std::make_unique<NodeText>(std::static_pointer_cast<LayoutEntityText>(shared_from_this()));
    }

    nlohmann::json LayoutEntityText::Serialize() const {
        nlohmann::json j = LayoutEntity::Serialize();
        j["m_fontName"] = m_fontName;
        j["m_fontSize"] = m_fontSize;
        j["m_text"] = m_text;
        j["m_horizontalAlignment"] = m_horizontalAlignment;
        j["m_verticalAlignment"] = m_verticalAlignment;
        return j;
    }

    void LayoutEntityText::Deserialize(nlohmann::json const& json) {
        LayoutEntity::Deserialize(json);
        json["m_fontName"].get_to(m_fontName);
        json["m_fontSize"].get_to(m_fontSize);
        json["m_text"].get_to(m_text);
        json["m_horizontalAlignment"].get_to(m_horizontalAlignment);
        json["m_verticalAlignment"].get_to(m_verticalAlignment);
    }
}
