#include "common.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/events/event_dispatch.h"
#include "moth_ui/nodes/node_clip.h"
#include "moth_ui/context.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/events/event_animation.h"

namespace moth_ui {
    Group::Group(Context& context)
        : Node(context) {
        m_animationClipController = std::make_unique<AnimationClipController>(this);
    }

    Group::Group(Context& context, std::shared_ptr<LayoutEntityGroup> layoutEntityGroup)
        : Node(context, layoutEntityGroup) {
        ReloadEntityPrivate();
    }

    bool Group::Broadcast(Event const& event) {
        // Children first (reverse z-order), then self. Deeper nodes get
        // first opportunity to handle the event during the depth-first walk.
        auto childrenCopy = m_children;
        for (auto it = std::rbegin(childrenCopy); it != std::rend(childrenCopy); ++it) {
            auto const child = *it;
            if (child->Broadcast(event)) {
                return true;
            }
        }
        return OnEvent(event);
    }

    void Group::Update(uint32_t ticks) {
        Node::Update(ticks);
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        m_animationClipController->Update(static_cast<float>(ticks) / 1000.0f);
        for (auto&& child : m_children) {
            child->Update(ticks);
        }
    }

    void Group::UpdateChildBounds() {
        for (auto&& child : m_children) {
            child->RecalculateBounds();
        }
    }

    void Group::AddChild(std::shared_ptr<Node> child, int index) {
        if (index >= 0 && static_cast<size_t>(index) <= m_children.size()) {
            auto it = std::begin(m_children) + static_cast<std::ptrdiff_t>(index);
            m_children.insert(it, child);
        } else {
            m_children.push_back(child);
        }

        child->SetParent(this);
    }

    void Group::RemoveChild(std::shared_ptr<Node> child) {
        auto it = ranges::find(m_children, child);
        if (std::end(m_children) != it) {
            (*it)->SetParent(nullptr);
            m_children.erase(it);
        }
    }

    int Group::IndexOf(std::shared_ptr<Node> child) const {
        auto const it = ranges::find(m_children, child);
        if (std::end(m_children) != it) {
            return static_cast<int>(it - std::begin(m_children));
        }
        return -1;
    }

    bool Group::HasAnimation(std::string_view const& name) {
        if (m_layout) {
            auto layout = std::static_pointer_cast<LayoutEntityGroup>(m_layout);
            auto& animationClips = layout->m_clips;
            auto it = ranges::find_if(animationClips, [&name](auto& clip) { return clip->m_name == name; });
            if (std::end(animationClips) != it) {
                return true;
            }
        }
        return false;
    }

    bool Group::SetAnimation(std::string_view const& name) {
        if (m_layout) {
            auto layout = std::static_pointer_cast<LayoutEntityGroup>(m_layout);
            auto& animationClips = layout->m_clips;
            auto it = ranges::find_if(animationClips, [&name](auto& clip) { return clip->m_name == name; });
            if (std::end(animationClips) != it) {
                m_animationClipController->SetClip(*it);
                return true;
            }
        }
        return false;
    }

    void Group::StopAnimation() {
        m_animationClipController->SetClip(nullptr);
    }

    std::shared_ptr<Node> Group::GetChild(std::string_view id) {
        auto const it = ranges::find_if(m_children, [&](auto child) { return child->GetId() == id; });
        if (std::end(m_children) != it) {
            return *it;
        }
        return nullptr;
    }

    std::shared_ptr<Node> Group::FindChild(std::string_view id) {
        if (id == m_id) {
            return shared_from_this();
        }
        std::shared_ptr<Node> found;
        for (auto&& child : m_children) {
            found = child->FindChild(id);
            if (found) {
                break;
            }
        }
        return found;
    }

    void Group::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        ReloadEntityPrivate();
    }

    void Group::DrawInternal() {
        for (auto it = std::rbegin(m_children); it != std::rend(m_children); ++it) {
            if (auto const clipNode = std::dynamic_pointer_cast<NodeClip>(*it)) {
                m_context.GetRenderer().PushClip(clipNode->GetScreenRect());
            }
        }
        for (auto&& child : m_children) {
            if (auto const clipNode = std::dynamic_pointer_cast<NodeClip>(child)) {
                m_context.GetRenderer().PopClip();
            } else {
                child->Draw();
            }
        }
    }

    void Group::ReloadEntityPrivate() {
        auto const layoutEntity = std::static_pointer_cast<LayoutEntityGroup>(m_layout);
        for (auto it = std::begin(m_children); it != std::end(m_children); /* skip */) {
            (*it)->SetParent(nullptr);
            it = m_children.erase(it);
        }
        auto& nodeFactory = NodeFactory::Get();
        for (auto&& childEntity : layoutEntity->m_children) {
            AddChild(nodeFactory.Create(m_context, childEntity));
        }
        UpdateChildBounds();
        m_animationClipController = std::make_unique<AnimationClipController>(this);
    }
}
