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

    std::shared_ptr<LayoutEntity> LayoutEntityText::Clone(CloneType cloneType) {
        return std::make_shared<LayoutEntityText>(*this);
    }

    std::unique_ptr<Node> LayoutEntityText::Instantiate(Context& context) {
        return std::make_unique<NodeText>(context, std::static_pointer_cast<LayoutEntityText>(shared_from_this()));
    }

    nlohmann::json LayoutEntityText::Serialize(SerializeContext const& context) const {
        nlohmann::json j = LayoutEntity::Serialize(context);
        j["fontName"] = m_fontName;
        j["fontSize"] = m_fontSize;
        j["text"] = m_text;
        j["horizontalAlignment"] = m_horizontalAlignment;
        j["verticalAlignment"] = m_verticalAlignment;
        j["dropShadow"] = m_dropShadow;
        j["dropShadowOffset"] = m_dropShadowOffset;
        j["dropShadowColor"] = m_dropShadowColor;
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
            m_dropShadow = json.value("dropShadow", false);
            m_dropShadowOffset = json.value("dropShadowOffset", IntVec2{ 0, 0 });
            m_dropShadowColor = json.value("dropShadowColor", BasicColors::Black);
        }

        return success;
    }

    nlohmann::json LayoutEntityText::SerializeOverrides() const {
        nlohmann::json j = LayoutEntity::SerializeOverrides();
        if (m_hardReference) {
            auto const textEntity = std::static_pointer_cast<LayoutEntityText>(m_hardReference);
            if (textEntity->m_text != m_text) {
                j["text"] = m_text;
            }
        }
        return j;
    }

    void LayoutEntityText::DeserializeOverrides(nlohmann::json const& overridesJson) {
        LayoutEntity::DeserializeOverrides(overridesJson);
        auto const textEntity = std::static_pointer_cast<LayoutEntityText>(m_hardReference);
        m_text = overridesJson.value("text", m_text);
    }
}
