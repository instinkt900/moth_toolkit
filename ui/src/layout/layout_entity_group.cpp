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

    LayoutEntityGroup::LayoutEntityGroup(LayoutEntityGroup const& other)
        : LayoutEntity(other) {
        for (auto&& child : other.m_children) {
            m_children.push_back(child->Clone(moth_ui::LayoutEntity::CloneType::Deep));
        }
        for (auto&& clip : other.m_clips) {
            m_clips.push_back(std::make_unique<AnimationClip>(*clip));
        }
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

    bool LayoutEntityGroup::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        assert(false && "Group should never be deserialized");
        return false;
    }
}
