#include "common.h"
#include "moth_ui/widgets/ui_button.h"

#include "moth_ui/events/event_dispatch.h"

namespace moth_ui {
    UIButton::UIButton(Context& context, std::shared_ptr<LayoutEntityGroup> entity)
        : Widget<UIButton>(context, std::move(entity)) {
    }

    void UIButton::Update(uint32_t ticks) {
        Group::Update(ticks);
    }

    bool UIButton::OnEvent(Event const& event) {
        EventDispatch dispatch(event);
        dispatch.Dispatch(this, &UIButton::OnMouseDown);
        dispatch.Dispatch(this, &UIButton::OnMouseUp);
        dispatch.Dispatch(this, &UIButton::OnMouseMove);
        return dispatch.GetHandled() || Group::OnEvent(event);
    }

    void UIButton::SetClickAction(std::function<void()> action) {
        m_clickAction = std::move(action);
    }

    bool UIButton::OnMouseDown(EventMouseDown const& event) {
        if (IsInBounds(event.GetPosition())) {
            SetState(State::Pressed);
            return true;
        }
        return false;
    }

    bool UIButton::OnMouseUp(EventMouseUp const& event) {
        if (IsInBounds(event.GetPosition()) && m_state == State::Pressed) {
            SetState(State::Activated);
        } else {
            SetState(State::Idle);
        }
        return false;
    }

    bool UIButton::OnMouseMove(EventMouseMove const& event) {
        if (IsInBounds(event.GetPosition())) {
            if (m_state != State::Pressed) {
                SetState(State::Hovered);
            }
        } else {
            if (m_state == State::Hovered) {
                SetState(State::Idle);
            }
        }
        return false;
    }

    void UIButton::SetState(State state) {
        if (m_state == state) {
            return;
        }
        m_state = state;
        switch (m_state) {
        default:
        case State::Idle:
            SetAnimation("idle");
            break;
        case State::Hovered:
            SetAnimation("hover");
            break;
        case State::Pressed:
            SetAnimation("press");
            break;
        case State::Activated:
            SetAnimation("activate");
            if (m_clickAction) {
                m_clickAction();
            }
            break;
        }
    }
}
