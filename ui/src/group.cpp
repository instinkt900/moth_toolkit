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
        for (auto&& childEntity : layoutEntityGroup->m_children) {
            AddChild(childEntity->Instantiate());
        }
    }

    Group::~Group() {
    }

    bool Group::OnEvent(Event const& event) {
        if (Node::OnEvent(event)) {
            return true;
        }
        EventDispatch dispatch(event);
        for (auto&& child : m_children) {
            dispatch.Dispatch(child.get());
        }
        return dispatch.GetHandled();
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

    void Group::DebugDraw() {
        Node::DebugDraw();
        //ImGuiInspectMember("anim time", m_animTime);
        if (ImGui::TreeNode("Children")) {
            for (int i = 0; i < m_children.size(); ++i) {
                auto& child = m_children[i];
                auto const label = fmt::format("[{}]: {}", i, child->GetId());
                if (ImGui::TreeNode(label.c_str())) {
                    child->DebugDraw();
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }

    void Group::DrawInternal() {
        bool popClip = false;
        if (m_clipRect && m_clipRect->IsVisible()) {
            Context::GetCurrentContext().GetRenderer().PushClip(m_clipRect->GetScreenRect());
            popClip = true;
        }

        for (auto&& child : m_children) {
            child->Draw();
        }

        if (popClip) {
            Context::GetCurrentContext().GetRenderer().PopClip();
        }
    }

}
