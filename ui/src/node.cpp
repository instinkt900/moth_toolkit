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
        : m_layout(layoutEntity)
        , m_id(m_layout->GetId())
        , m_layoutRect(m_layout->GetBoundsAtFrame(0))
        , m_color(m_layout->GetColorAtFrame(0))
        , m_blend(m_layout->GetBlendMode()) {
        m_animationController = std::make_unique<AnimationController>(this, m_layout->GetAnimationTracks());
    }

    Node::~Node() {
    }

    bool Node::OnEvent(Event const& event) {
        return false;
    }

    void Node::Update(uint32_t ticks) {
        m_animationController->Update(ticks / 1000.0f);
    }

    void Node::Draw() {
        if (!IsVisible()) {
            return;
        }

        Context::GetCurrentContext().GetRenderer().PushBlendMode(m_blend);
        Context::GetCurrentContext().GetRenderer().PushColor(m_color);
        DrawInternal();
        Context::GetCurrentContext().GetRenderer().PopColor();
        Context::GetCurrentContext().GetRenderer().PopBlendMode();

        if (m_showRect) {
            auto& renderer = Context::GetCurrentContext().GetRenderer();
            renderer.DrawRect(m_screenRect, Color::Red, BlendMode::Replace);
        }
    }

    void Node::SendEvent(Event const& event) {
        if (m_eventHandler && m_eventHandler(this, event)) {
            return;
        }
        if (m_parent) {
            m_parent->SendEvent(event);
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
        m_id = m_layout->GetId();
        m_layoutRect = m_layout->GetBoundsAtFrame(0);
        m_color = m_layout->GetColorAtFrame(0);
        m_animationController = std::make_unique<AnimationController>(this, m_layout->GetAnimationTracks());
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
}
