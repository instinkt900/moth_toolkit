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

    nlohmann::json LayoutEntityGroup::Serialize(SerializeContext const& context) const {
        assert(false && "Group should never be serialized");
        return {};
    }

    void LayoutEntityGroup::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        assert(false && "Group should never be deserialized");
    }
}
