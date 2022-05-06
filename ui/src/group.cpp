#include "common.h"
#include "moth_ui/group.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/event_dispatch.h"
#include "moth_ui/node_clip.h"
#include "moth_ui/context.h"

namespace moth_ui {
    Group::Group() {
    }

    Group::Group(std::shared_ptr<LayoutEntityGroup> layoutEntityGroup)
        : Node(layoutEntityGroup) {
        ReloadEntityPrivate();
    }

    Group::~Group() {
    }

    bool Group::SendEventDown(Event const& event) {
        // disabled/visible checks?

        if (OnEvent(event)) {
            return true;
        }

        for (auto&& child : m_children) {
            if (child->SendEventDown(event)) {
                return true;
            }
        }

        return false;
    }

    void Group::Update(uint32_t ticks) {
        Node::Update(ticks);
        for (auto&& child : m_children) {
            child->Update(ticks);
        }
    }

    void Group::UpdateChildBounds() {
        for (auto&& child : m_children) {
            child->RecalculateBounds();
        }
    }

    void Group::AddChild(std::shared_ptr<Node> child) {
        m_children.push_back(child);
        child->SetParent(this);

        if (auto const clipNode = std::dynamic_pointer_cast<NodeClip>(child)) {
            m_clipRect = clipNode.get();
        }
    }

    void Group::RemoveChild(std::shared_ptr<Node> child) {
        if (child.get() == m_clipRect) {
            m_clipRect = nullptr;
        }

        auto it = ranges::find(m_children, child);
        if (std::end(m_children) != it) {
            (*it)->SetParent(nullptr);
            m_children.erase(it);
        }
    }

    std::shared_ptr<Node> Group::FindChild(std::string const& childId) {
        auto const it = ranges::find_if(m_children, [&](auto child) { return child->GetId() == childId; });
        if (std::end(m_children) != it) {
            return *it;
        }
        return nullptr;
    }

    bool Group::SetAnimation(std::string const& name) {
        if (m_layout) {
            auto layout = std::static_pointer_cast<LayoutEntityGroup>(m_layout);
            auto& animationClips = layout->m_clips;
            auto it = ranges::find_if(animationClips, [&name](auto& clip) { return clip->m_name == name; });
            if (std::end(animationClips) != it) {
                for (auto&& child : m_children) {
                    child->SetAnimationClip(it->get());
                }
                return true;
            }
        }
        return false;
    }

    void Group::StopAnimation() {
        for (auto&& child : m_children) {
            child->SetAnimationClip(nullptr);
        }
    }

    void Group::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        ReloadEntityPrivate();
    }

    void Group::DrawInternal() {
        bool popClip = false;
        if (m_clipRect && m_clipRect->IsVisible()) {
            Context::GetCurrentContext()->GetRenderer().PushClip(m_clipRect->GetScreenRect());
            popClip = true;
        }

        for (auto&& child : m_children) {
            child->Draw();
        }

        if (popClip) {
            Context::GetCurrentContext()->GetRenderer().PopClip();
        }
    }

    void Group::ReloadEntityPrivate() {
        auto const layoutEntity = std::static_pointer_cast<LayoutEntityGroup>(m_layout);
        m_children.clear();
        auto& nodeFactory = Context::GetCurrentContext()->GetNodeFactory();
        for (auto&& childEntity : layoutEntity->m_children) {
            AddChild(nodeFactory.CreateNode(childEntity));
        }
    }
}
