#include "common.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/animation/animation_event.h"

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
        for (auto&& event : other.m_events) {
            m_events.push_back(std::make_unique<AnimationEvent>(*event));
        }
    }

    LayoutEntityGroup::LayoutEntityGroup(LayoutEntityGroup&& other) noexcept
        : LayoutEntity(other) {
        for (auto&& child : other.m_children) {
            m_children.push_back(child->Clone(moth_ui::LayoutEntity::CloneType::Deep));
        }
        for (auto&& clip : other.m_clips) {
            m_clips.push_back(std::make_unique<AnimationClip>(*clip));
        }
        for (auto&& event : other.m_events) {
            m_events.push_back(std::make_unique<AnimationEvent>(*event));
        }
    }

    LayoutEntityGroup::~LayoutEntityGroup() = default;

    nlohmann::json LayoutEntityGroup::Serialize(SerializeContext const& context) const {
        assert(false && "Group should never be serialized");
        return {};
    }

    bool LayoutEntityGroup::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        assert(false && "Group should never be deserialized");
        return false;
    }
}
