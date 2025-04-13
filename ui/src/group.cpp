#include "common.h"
#include "moth_ui/group.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/event_dispatch.h"
#include "moth_ui/node_clip.h"
#include "moth_ui/context.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/events/event_animation.h"

namespace moth_ui {
    Group::Group(Context& context)
        : Node(context) {
    }

    Group::Group(Context& context, std::shared_ptr<LayoutEntityGroup> layoutEntityGroup)
        : Node(context, layoutEntityGroup) {
        ReloadEntityPrivate();
    }

    Group::~Group() {
    }

    bool Group::SendEventDown(Event const& event) {
        // disabled/visible checks?

        if (OnEvent(event)) {
            return true;
        }

        auto childrenCopy = m_children;
        for (auto&& child : childrenCopy) {
            if (child->SendEventDown(event)) {
                return true;
            }
        }

        return false;
    }

    void Group::Update(uint32_t ticks) {
        Node::Update(ticks);
        m_animationClipController->Update(ticks / 1000.0f);
        for (auto&& child : m_children) {
            child->Update(ticks);
        }
    }

    void Group::UpdateChildBounds() {
        for (auto&& child : m_children) {
            child->RecalculateBounds();
        }
    }

    void Group::AddChild(std::shared_ptr<Node> child, size_t index) {
        if (index != static_cast<size_t>(-1)) {
            auto it = std::begin(m_children) + index;
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

    size_t Group::IndexOf(std::shared_ptr<Node> child) const {
        auto const it = ranges::find(m_children, child);
        if (std::end(m_children) != it) {
            return it - std::begin(m_children);
        }
        return static_cast<size_t>(-1);
    }

    bool Group::SetAnimation(std::string const& name) {
        if (m_layout) {
            auto layout = std::static_pointer_cast<LayoutEntityGroup>(m_layout);
            auto& animationClips = layout->m_clips;
            auto it = ranges::find_if(animationClips, [&name](auto& clip) { return clip->m_name == name; });
            if (std::end(animationClips) != it) {
                m_animationClipController->SetClip(it->get());
                return true;
            }
        }
        return false;
    }

    void Group::StopAnimation() {
        m_animationClipController->SetClip(nullptr);
    }

    std::shared_ptr<Node> Group::GetChild(std::string const& id) {
        auto const it = ranges::find_if(m_children, [&](auto child) { return child->GetId() == id; });
        if (std::end(m_children) != it) {
            return *it;
        }
        return nullptr;
    }

    std::shared_ptr<Node> Group::FindChild(std::string const& id) {
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
        int clipRects = 0;
        for (auto&& child : m_children) {
            if (auto const clipNode = std::dynamic_pointer_cast<NodeClip>(child)) {
                m_context.GetRenderer().PushClip(clipNode->GetScreenRect());
                ++clipRects;
            }
            child->Draw();
        }

        while (clipRects--) {
            m_context.GetRenderer().PopClip();
        }
    }

    void Group::ReloadEntityPrivate() {
        auto const layoutEntity = std::static_pointer_cast<LayoutEntityGroup>(m_layout);
        m_children.clear();
        auto& nodeFactory = NodeFactory::Get();
        for (auto&& childEntity : layoutEntity->m_children) {
            AddChild(nodeFactory.Create(m_context, childEntity));
        }
        UpdateChildBounds();
        m_animationClipController = std::make_unique<AnimationClipController>(this);
    }
}
