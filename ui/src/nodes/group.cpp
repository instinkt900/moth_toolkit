#include "common.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/layout/layout_entity_ref.h"
#include "moth_ui/nodes/node_clip.h"
#include "moth_ui/context.h"
#include "moth_ui/node_factory.h"

namespace moth_ui {
    Group::Group(Context& context)
        : Node(context) {
        m_animationClipController = std::make_unique<AnimationClipController>(this);
    }

    Group::Group(Context& context, std::shared_ptr<LayoutEntityGroup> layoutEntityGroup)
        : Node(context, layoutEntityGroup)
        , m_typedLayout(layoutEntityGroup.get()) {
        ReloadChildren();
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

    void Group::MoveChild(int fromIndex, int toIndex) {
        if (fromIndex < 0 || toIndex < 0
            || static_cast<size_t>(fromIndex) >= m_children.size()
            || static_cast<size_t>(toIndex) >= m_children.size()) {
            return;
        }
        auto child = m_children[fromIndex];
        m_children.erase(std::next(std::begin(m_children), fromIndex));
        m_children.insert(std::next(std::begin(m_children), toIndex), std::move(child));
    }

    bool Group::HasAnimation(std::string_view name) const {
        if (m_layout) {
            auto& animationClips = m_typedLayout->m_clips;
            auto it = ranges::find_if(animationClips, [&name](auto& clip) { return clip->name == name; });
            if (std::end(animationClips) != it) {
                return true;
            }
        }
        return false;
    }

    bool Group::SetAnimation(std::string_view name) {
        if (m_layout) {
            auto& animationClips = m_typedLayout->m_clips;
            auto it = ranges::find_if(animationClips, [&name](auto& clip) { return clip->name == name; });
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
        ReloadChildren();
        UpdateChildBounds();
    }

    void Group::ReloadChildren() {
        for (auto& child : m_children) {
            child->SetParent(nullptr);
        }
        m_children.clear();
        auto& nodeFactory = NodeFactory::Get();
        for (auto&& childEntity : m_typedLayout->m_children) {
            AddChild(nodeFactory.Create(m_context, childEntity));
        }
        m_animationClipController = std::make_unique<AnimationClipController>(this);
    }

    void Group::ReapplyOverrides(LayoutEntity& childLayout) {
        if (auto* ref = dynamic_cast<LayoutEntityRef*>(m_layout.get())) {
            ref->ReapplyOverrides(childLayout);
        }
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

    std::shared_ptr<Group> Group::Create(Context& context) {
        return std::shared_ptr<Group>(new Group(context));
    }

    std::shared_ptr<Group> Group::Create(Context& context, std::shared_ptr<LayoutEntityGroup> layoutEntityGroup) {
        if (!layoutEntityGroup) {
            return nullptr;
        }
        auto group = std::shared_ptr<Group>(new Group(context, std::move(layoutEntityGroup)));
        group->UpdateChildBounds();
        return group;
    }
}
