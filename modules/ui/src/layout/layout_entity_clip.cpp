#include "common.h"
#include "moth/ui/layout/layout_entity_clip.h"
#include "moth/ui/nodes/node_clip.h"
#include "moth/ui/animation/animation_controller.h"

namespace moth::ui {
    LayoutEntityClip::LayoutEntityClip(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityClip::LayoutEntityClip(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
    }

    std::shared_ptr<LayoutEntity> LayoutEntityClip::Clone(CloneType cloneType) {
        return std::make_shared<LayoutEntityClip>(*this);
    }

    std::shared_ptr<Node> LayoutEntityClip::Instantiate(Context& context) {
        return NodeClip::Create(context, std::static_pointer_cast<LayoutEntityClip>(shared_from_this()));
    }

    nlohmann::json LayoutEntityClip::Serialize(SerializeContext const& context) const {
        return LayoutEntity::Serialize(context);
    }

    bool LayoutEntityClip::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        return LayoutEntity::Deserialize(json, context);
    }
}
