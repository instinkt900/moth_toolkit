#include "common.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/animation/animation_marker.h"

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
            m_clips.push_back(std::make_shared<AnimationClip>(*clip));
        }
        for (auto&& event : other.m_events) {
            m_events.push_back(std::make_unique<AnimationMarker>(*event));
        }
    }

    LayoutEntityGroup::LayoutEntityGroup(LayoutEntityGroup&& other) noexcept
        : LayoutEntity(std::move(other))
        , m_children(std::move(other.m_children))
        , m_clips(std::move(other.m_clips))
        , m_events(std::move(other.m_events)) {
        for (auto& child : m_children) {
            child->m_parent = this;
        }
    }

    LayoutEntityGroup::~LayoutEntityGroup() = default;
}
