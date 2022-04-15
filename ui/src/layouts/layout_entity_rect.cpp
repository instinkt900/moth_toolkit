#include "common.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/node_rect.h"

namespace moth_ui {
    LayoutEntityRect::LayoutEntityRect(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityRect::LayoutEntityRect(nlohmann::json const& json, LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
        Deserialize(json);
    }

    std::unique_ptr<Node> LayoutEntityRect::Instantiate() {
        return std::make_unique<NodeRect>(std::static_pointer_cast<LayoutEntityRect>(shared_from_this()));
    }

    nlohmann::json LayoutEntityRect::Serialize() const {
        nlohmann::json j = LayoutEntity::Serialize();
        j["m_filled"] = m_filled;
        return j;
    }

    void LayoutEntityRect::Deserialize(nlohmann::json const& json) {
        LayoutEntity::Deserialize(json);
        json.at("m_filled").get_to(m_filled);
    }
}
