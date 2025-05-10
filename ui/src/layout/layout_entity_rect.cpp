#include "common.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/nodes/node_rect.h"
#include "moth_ui/animation/animation_controller.h"

namespace moth_ui {
    LayoutEntityRect::LayoutEntityRect(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityRect::LayoutEntityRect(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
    }

    std::shared_ptr<LayoutEntity> LayoutEntityRect::Clone(CloneType cloneType) {
        return std::make_shared<LayoutEntityRect>(*this);
    }

    std::unique_ptr<Node> LayoutEntityRect::Instantiate(Context& context) {
        return std::make_unique<NodeRect>(context, std::static_pointer_cast<LayoutEntityRect>(shared_from_this()));
    }

    nlohmann::json LayoutEntityRect::Serialize(SerializeContext const& context) const {
        nlohmann::json j = LayoutEntity::Serialize(context);
        j["filled"] = m_filled;
        return j;
    }

    bool LayoutEntityRect::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        bool success = LayoutEntity::Deserialize(json, context);

        if (success) {
            m_filled = json.value("filled", true);
        }

        return success;
    }
}
