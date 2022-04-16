#include "common.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/node_rect.h"

namespace moth_ui {
    LayoutEntityRect::LayoutEntityRect(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityRect::LayoutEntityRect(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
    }

    std::unique_ptr<Node> LayoutEntityRect::Instantiate() {
        return std::make_unique<NodeRect>(std::static_pointer_cast<LayoutEntityRect>(shared_from_this()));
    }

    nlohmann::json LayoutEntityRect::Serialize() const {
        nlohmann::json j = LayoutEntity::Serialize();
        j["filled"] = m_filled;
        return j;
    }

    void LayoutEntityRect::Deserialize(nlohmann::json const& json, int dataVersion) {
        LayoutEntity::Deserialize(json, dataVersion);
        json.at("filled").get_to(m_filled);
    }
}
