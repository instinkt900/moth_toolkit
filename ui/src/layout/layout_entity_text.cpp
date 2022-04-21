#include "common.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/node_text.h"

namespace moth_ui {
    LayoutEntityText::LayoutEntityText(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityText::LayoutEntityText(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
    }

    std::unique_ptr<Node> LayoutEntityText::Instantiate() {
        return std::make_unique<NodeText>(std::static_pointer_cast<LayoutEntityText>(shared_from_this()));
    }

    nlohmann::json LayoutEntityText::Serialize(SerializeContext const& context) const {
        nlohmann::json j = LayoutEntity::Serialize(context);
        j["fontName"] = m_fontName;
        j["fontSize"] = m_fontSize;
        j["text"] = m_text;
        j["horizontalAlignment"] = m_horizontalAlignment;
        j["verticalAlignment"] = m_verticalAlignment;
        return j;
    }

    void LayoutEntityText::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        LayoutEntity::Deserialize(json, context);
        json["fontName"].get_to(m_fontName);
        json["fontSize"].get_to(m_fontSize);
        json["text"].get_to(m_text);
        json["horizontalAlignment"].get_to(m_horizontalAlignment);
        json["verticalAlignment"].get_to(m_verticalAlignment);
    }
}
