#include "common.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/group.h"
#include "moth_ui/animation_clip.h"

namespace moth_ui {
    LayoutEntityGroup::LayoutEntityGroup(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityGroup::LayoutEntityGroup(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
    }

    void LayoutEntityGroup::RefreshAnimationTimings() {
        LayoutEntity::RefreshAnimationTimings();
        for (auto&& child : m_children) {
            child->RefreshAnimationTimings();
        }
    }

    std::unique_ptr<Node> LayoutEntityGroup::Instantiate() {
        return std::make_unique<Group>(std::static_pointer_cast<LayoutEntityGroup>(shared_from_this()));
    }

    nlohmann::json LayoutEntityGroup::Serialize() const {
        assert(false && "Group should never be serialized");
        return {};
    }

    void LayoutEntityGroup::Deserialize(nlohmann::json const& json, int dataVersion) {
        assert(false && "Group should never be deserialized");
    }
}
