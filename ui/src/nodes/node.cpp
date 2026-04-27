#include "common.h"
#include "moth_ui/nodes/node.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/animation/animation_controller.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/context.h"

namespace moth_ui {
    Node::Node(Context& context)
        : m_context(context) {
        m_animationController = std::make_unique<AnimationController>(*this);
    }

    Node::~Node() = default;

    Node::Node(Context& context, std::shared_ptr<LayoutEntity> layoutEntity)
        : m_context(context)
        , m_layout(layoutEntity) {
        Node::ReloadEntityInternal();
    }

    bool Node::SendEvent(Event const& event, EventDirection direction) {
        if (direction == EventDirection::Up) {
            return SendEventUp(event);
        }
        if (direction == EventDirection::Down) {
            return Broadcast(event);
        }
        assert(false && "Bad event direction.");
        return false;
    }

    bool Node::SendEventUp(Event const& event) {
        auto* currentNode = this;
        while (currentNode != nullptr) {
            if (currentNode->OnEvent(event)) {
                return true;
            }
            currentNode = currentNode->GetParent();
        }
        return false;
    }

    bool Node::Broadcast(Event const& event) {
        return OnEvent(event);
    }

    bool Node::OnEvent(Event const& event) {
        if (m_eventHandler) {
            return m_eventHandler(this, event);
        }
        return false;
    }

    void Node::Update(uint32_t ticks) {
    }

    void Node::UpdateLocalTransform() {
        float const w = static_cast<float>(m_screenRect.bottomRight.x - m_screenRect.topLeft.x);
        float const h = static_cast<float>(m_screenRect.bottomRight.y - m_screenRect.topLeft.y);
        FloatVec2 const localPivot = { m_pivot.x * w, m_pivot.y * h };
        FloatVec2 const translation = m_parent != nullptr
            ? static_cast<FloatVec2>(m_screenRect.topLeft - m_parent->GetScreenRect().topLeft)
            : static_cast<FloatVec2>(m_screenRect.topLeft);
        m_localTransform = FloatMat4x4::Translation(translation) * FloatMat4x4::Rotation(m_rotation, localPivot);
    }

    FloatMat4x4 Node::GetWorldTransform() const {
        if (m_parent != nullptr) {
            return m_parent->GetWorldTransform() * m_localTransform;
        }
        return m_localTransform;
    }

    void Node::SetRotation(float rotation) {
        m_rotation = rotation;
        UpdateLocalTransform();
    }

    void Node::SetPivot(FloatVec2 const& pivot) {
        m_pivot = pivot;
        UpdateLocalTransform();
    }

    void Node::Draw() {
        if (!IsVisible()) {
            return;
        }

        float const w = static_cast<float>(m_screenRect.bottomRight.x - m_screenRect.topLeft.x);
        float const h = static_cast<float>(m_screenRect.bottomRight.y - m_screenRect.topLeft.y);

        auto& renderer = m_context.GetRenderer();
        renderer.PushTransform(GetWorldTransform());
        renderer.PushBlendMode(m_blend);
        renderer.PushColor(m_color);
        DrawInternal();
        if (m_showRect) {
            renderer.PushColor(BasicColors::Red);
            renderer.PushBlendMode(BlendMode::Replace);
            renderer.RenderRect(IntRect{ { 0, 0 }, { static_cast<int>(w), static_cast<int>(h) } });
            renderer.PopBlendMode();
            renderer.PopColor();
        }
        renderer.PopColor();
        renderer.PopBlendMode();
        renderer.PopTransform();
    }

    void Node::SetScreenRect(IntRect const& rect) {
        m_overrideScreenRect = true;
        if (m_screenRect != rect) {
            m_screenRect = rect;
            UpdateLocalTransform();
            UpdateChildBounds();
        }
    }

    void Node::Refresh(float frame) {
        m_layoutRect = m_layout->GetBoundsAtFrame(frame);
        m_color = m_layout->GetColorAtFrame(frame);
        m_rotation = m_layout->GetRotationAtFrame(frame);
        m_animationController->SetFrameDiscrete(frame);
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
        UpdateLocalTransform();
        UpdateChildBounds();
    }

    void Node::ReloadEntity() {
        ReloadEntityInternal();

        if (m_parent != nullptr && m_layout) {
            m_parent->ReapplyOverrides(*m_layout);
            m_visible = m_layout->m_visible;
            m_blend = m_layout->m_blend;
        }
    }

    bool Node::IsInBounds(IntVec2 const& point) const {
        auto const localPoint = GetWorldTransform().Invert().TransformPoint(static_cast<FloatVec2>(point));
        float const w = static_cast<float>(m_screenRect.bottomRight.x - m_screenRect.topLeft.x);
        float const h = static_cast<float>(m_screenRect.bottomRight.y - m_screenRect.topLeft.y);
        return localPoint.x >= 0.0f && localPoint.x <= w && localPoint.y >= 0.0f && localPoint.y <= h;
    }

    IntVec2 Node::TranslatePosition(IntVec2 const& point) const {
        auto const localPoint = GetWorldTransform().Invert().TransformPoint(static_cast<FloatVec2>(point));
        return static_cast<IntVec2>(localPoint);
    }

    std::shared_ptr<Node> Node::FindChild(std::string_view id) {
        if (id == m_id) {
            return shared_from_this();
        }
        return nullptr;
    }

    void Node::ReloadEntityInternal() {
        m_id = m_layout->m_id;
        m_layoutRect = m_layout->GetBoundsAtFrame(0);
        m_color = m_layout->GetColorAtFrame(0);
        m_rotation = m_layout->GetRotationAtFrame(0);
        m_pivot = m_layout->m_pivot;
        m_visible = m_layout->m_visible;
        m_blend = m_layout->m_blend;
        m_animationController = std::make_unique<AnimationController>(*this);
        UpdateLocalTransform();
    }
}
