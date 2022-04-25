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

    std::shared_ptr<LayoutEntity> LayoutEntityText::Clone() {
        return std::make_shared<LayoutEntityText>(*this);
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

    bool LayoutEntityText::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        bool success = LayoutEntity::Deserialize(json, context);

        if (success) {
            m_fontName = json.value("fontName", "");
            m_fontSize = json.value("fontSize", 10);
            m_text = json.value("text", "");
            m_horizontalAlignment = json.value("horizontalAlignment", TextHorizAlignment::Left);
            m_verticalAlignment = json.value("verticalAlignment", TextVertAlignment::Top);
        }

        return success;
    }
}
