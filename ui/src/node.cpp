#include "common.h"
#include "moth_ui/node.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/animation_controller.h"
#include "moth_ui/group.h"
#include "moth_ui/utils/imgui_ext_inspect.h"
#include "moth_ui/context.h"

namespace moth_ui {
    Node::Node() {
    }

    Node::Node(std::shared_ptr<LayoutEntity> layoutEntity)
        : m_layout(layoutEntity) {
        ReloadEntityInternal();
    }

    Node::~Node() {
    }

    bool Node::SendEvent(Event const& event, EventDirection direction) {
        if (direction == EventDirection::Up) {
            return SendEventUp(event);
        } else if (direction == EventDirection::Down) {
            return SendEventDown(event);
        }
        assert(false && "Bad event direction.");
        return false;
    }

    bool Node::SendEventUp(Event const& event) {
        auto currentNode = this;
        do {
            if (currentNode->OnEvent(event)) {
                return true;
            }
            currentNode = currentNode->GetParent();
        } while (currentNode != nullptr);
        return false;
    }

    bool Node::SendEventDown(Event const& event) {
        return OnEvent(event);
    }

    bool Node::OnEvent(Event const& event) {
        if (m_eventHandler) {
            return m_eventHandler(this, event);
        }
        return false;
    }

    void Node::Update(uint32_t ticks) {
        m_animationController->Update(ticks / 1000.0f);
    }

    void Node::Draw() {
        if (!IsVisible()) {
            return;
        }

        auto& renderer = Context::GetCurrentContext()->GetRenderer();
        renderer.PushBlendMode(m_blend);
        renderer.PushColor(m_color);
        DrawInternal();
        renderer.PopColor();
        renderer.PopBlendMode();

        if (m_showRect) {
            renderer.PushColor(BasicColors::Red);
            renderer.PushBlendMode(BlendMode::Replace);
            renderer.RenderRect(m_screenRect);
            renderer.PopBlendMode();
            renderer.PopColor();
        }
    }

    void Node::SetScreenRect(IntRect const& rect) {
        m_overrideScreenRect = true;
        if (m_screenRect != rect) {
            m_screenRect = rect;
            UpdateChildBounds();
        }
    }

    void Node::Refresh(int frameNo) {
        m_layoutRect = m_layout->GetBoundsAtFrame(frameNo);
        m_color = m_layout->GetColorAtFrame(frameNo);
        RecalculateBounds();
    }

    void Node::RecalculateBounds() {
        if (nullptr != m_parent && !m_overrideScreenRect) {
            IntRect const& parentBounds = m_parent->GetScreenRect();
            FloatVec2 const parentOffset = static_cast<FloatVec2>(parentBounds.topLeft);
            FloatVec2 const parentDimensions = static_cast<FloatVec2>(parentBounds.bottomRight - parentBounds.topLeft);
            m_screenRect.topLeft = static_cast<IntVec2>(parentOffset + m_layoutRect.offset.topLeft + parentDimensions * m_layoutRect.anchor.topLeft);
            m_screenRect.bottomRight = static_cast<IntVec2>(parentOffset + m_layoutRect.offset.bottomRight + parentDimensions * m_layoutRect.anchor.bottomRight);
        }
        UpdateChildBounds();
    }

    void Node::ReloadEntity() {
        ReloadEntityInternal();

        // if our parent is a ref, check our overloads.
        // TODO
    }

    bool Node::IsInBounds(IntVec2 const& point) const {
        return IsInRect(point, m_screenRect);
    }

    IntVec2 Node::TranslatePosition(IntVec2 const& point) const {
        IntVec2 translated = point;
        translated.x -= m_screenRect.topLeft.x;
        translated.y -= m_screenRect.topLeft.y;
        return translated;
    }

    void Node::SetAnimationClip(AnimationClip* clip) {
        m_animationController->SetClip(clip);
    }

    void Node::UpdateAnimTime(float delta) {
        m_animationController->Update(delta);
        RecalculateBounds();
    }

    void Node::DebugDraw() {
        if (ImGui::TreeNode("Node")) {
            imgui_ext::Inspect("id", m_id);
            imgui_ext::Inspect("visible", m_visible);
            imgui_ext::Inspect("show rect", m_showRect);
            imgui_ext::Inspect("override bounds", m_overrideScreenRect);
            imgui_ext::Inspect("color", m_color);
            bool boundsChanged = false;
            boundsChanged |= imgui_ext::Inspect("bounds", m_layoutRect);
            boundsChanged |= imgui_ext::Inspect("screen rect", m_screenRect);
            if (boundsChanged) {
                RecalculateBounds();
            }
            ImGui::TreePop();
        }
    }

    void Node::ReloadEntityInternal() {
        ReloadEntityPrivate();
    }

    void Node::ReloadEntityPrivate() {
        m_id = m_layout->m_id;
        m_layoutRect = m_layout->GetBoundsAtFrame(0);
        m_color = m_layout->GetColorAtFrame(0);
        m_blend = m_layout->m_blend;
        m_animationController = std::make_unique<AnimationController>(this, m_layout->m_tracks);
    }
}
