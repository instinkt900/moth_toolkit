#pragma once

#include "moth_ui/events/event_mouse.h"
#include "moth_ui/flow/iclickable.h"
#include "moth_ui/widgets/widget.h"

namespace moth_ui {
    /**
     * @brief Opt-in clickable button widget for moth_ui layouts.
     *
     * Recognises @c "idle", @c "hover", @c "press", and @c "activate" clips
     * defined on its layout entity and switches between them in response to
     * pointer input. Implements @ref IClickable so the @ref flow::Flow
     * runtime (or any other consumer) can wire a callback that fires on the
     * transition into the @c Activated state.
     *
     * Apps that need richer interaction (focus rings, controller navigation,
     * audio cues) can implement their own widget that satisfies @ref IClickable
     * and ignore this type entirely.
     */
    class UIButton : public Widget<UIButton>, public IClickable {
    public:
        /// @brief Class name under which this widget self-registers with @ref NodeFactory.
        inline static char const* ClassName = "button";

        /// @brief Visual / interaction state of the button.
        enum class State {
            Idle,       ///< Pointer not over the button.
            Hovered,    ///< Pointer over the button, not pressed.
            Pressed,    ///< Pointer pressed down inside the button.
            Activated,  ///< Pointer released inside the button while pressed (click).
        };

        /**
         * @brief Constructs a UIButton from a layout entity group.
         * @param context Active rendering context.
         * @param entity  Layout entity describing this button.
         */
        UIButton(Context& context, std::shared_ptr<LayoutEntityGroup> entity);
        UIButton(UIButton const&) = delete;
        UIButton(UIButton&&) = delete;
        UIButton& operator=(UIButton const&) = delete;
        UIButton& operator=(UIButton&&) = delete;
        ~UIButton() override = default;

        /**
         * @brief Advances per-frame logic.
         * @param ticks Elapsed time in milliseconds.
         */
        void Update(uint32_t ticks) override;

        /**
         * @brief Dispatches input events to the button's typed handlers.
         * @param event Event to process.
         * @return @c true if the event was consumed.
         */
        bool OnEvent(Event const& event) override;

        /// @copydoc IClickable::SetClickAction
        void SetClickAction(std::function<void()> action) override;

        /// @brief Returns the button's current interaction state.
        State GetState() const { return m_state; }

    private:
        bool OnMouseDown(EventMouseDown const& event);
        bool OnMouseUp(EventMouseUp const& event);
        bool OnMouseMove(EventMouseMove const& event);

        void SetState(State state);

        State m_state = State::Idle;
        std::function<void()> m_clickAction;
    };
}
