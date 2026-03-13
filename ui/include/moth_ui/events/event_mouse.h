#pragma once

#include "moth_ui/events/event.h"
#include "moth_ui/utils/vector.h"

#include <memory>

namespace moth_ui {
    /// @brief Identifies a mouse button.
    enum class MouseButton {
        Unknown, ///< Unrecognised or unset button.
        Left,    ///< Primary (left) mouse button.
        Middle,  ///< Middle mouse button / scroll wheel click.
        Right,   ///< Secondary (right) mouse button.
    };

    /**
     * @brief Event fired when a mouse button is pressed.
     */
    class EventMouseDown : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param button   Which mouse button was pressed.
         * @param position Cursor position in screen space at the time of the press.
         */
        EventMouseDown(MouseButton button, IntVec2 const& position)
            : Event(GetStaticType())
            , m_button(button)
            , m_position(position) {}

        /// @brief Returns the static type code for EventMouseDown.
        static constexpr int GetStaticType() { return EVENTTYPE_MOUSE_DOWN; }

        /// @brief Returns which mouse button was pressed.
        MouseButton GetButton() const { return m_button; }

        /// @brief Returns the cursor position at the time of the press.
        IntVec2 const& GetPosition() const { return m_position; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventMouseDown>(m_button, m_position);
        }

        EventMouseDown(EventMouseDown const&) = default;
        EventMouseDown(EventMouseDown&&) = default;
        EventMouseDown& operator=(EventMouseDown const&) = default;
        EventMouseDown& operator=(EventMouseDown&&) = default;
        ~EventMouseDown() override {}

    private:
        MouseButton m_button;
        IntVec2 m_position;
    };

    /**
     * @brief Event fired when a mouse button is released.
     */
    class EventMouseUp : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param button   Which mouse button was released.
         * @param position Cursor position in screen space at the time of the release.
         */
        EventMouseUp(MouseButton button, IntVec2 const& position)
            : Event(GetStaticType())
            , m_button(button)
            , m_position(position) {}

        /// @brief Returns the static type code for EventMouseUp.
        static constexpr int GetStaticType() { return EVENTTYPE_MOUSE_UP; }

        /// @brief Returns which mouse button was released.
        MouseButton GetButton() const { return m_button; }

        /// @brief Returns the cursor position at the time of the release.
        IntVec2 const& GetPosition() const { return m_position; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventMouseUp>(m_button, m_position);
        }

        EventMouseUp(EventMouseUp const&) = default;
        EventMouseUp(EventMouseUp&&) = default;
        EventMouseUp& operator=(EventMouseUp const&) = default;
        EventMouseUp& operator=(EventMouseUp&&) = default;
        ~EventMouseUp() override {}

    private:
        MouseButton m_button;
        IntVec2 m_position;
    };

    /**
     * @brief Event fired when the mouse cursor moves.
     */
    class EventMouseMove : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param position New cursor position in screen space.
         * @param delta    Relative movement since the last move event.
         */
        EventMouseMove(IntVec2 const& position, FloatVec2 const& delta)
            : Event(GetStaticType())
            , m_position(position)
            , m_delta(delta) {}

        /// @brief Returns the static type code for EventMouseMove.
        static constexpr int GetStaticType() { return EVENTTYPE_MOUSE_MOVE; }

        /// @brief Returns the new cursor position in screen space.
        IntVec2 const& GetPosition() const { return m_position; }

        /// @brief Returns the relative cursor movement since the last event.
        FloatVec2 const& GetDelta() const { return m_delta; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventMouseMove>(m_position, m_delta);
        }

        EventMouseMove(EventMouseMove const&) = default;
        EventMouseMove(EventMouseMove&&) = default;
        EventMouseMove& operator=(EventMouseMove const&) = default;
        EventMouseMove& operator=(EventMouseMove&&) = default;
        ~EventMouseMove() override {}

    private:
        IntVec2 m_position;
        FloatVec2 m_delta;
    };

    /**
     * @brief Event fired when the mouse scroll wheel is moved.
     */
    class EventMouseWheel : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param delta Scroll amount, positive values scroll up/forward.
         */
        EventMouseWheel(IntVec2 const& delta)
            : Event(GetStaticType())
            , m_delta(delta) {}

        /// @brief Returns the static type code for EventMouseWheel.
        static constexpr int GetStaticType() { return EVENTTYPE_MOUSE_WHEEL; }

        /// @brief Returns the scroll delta.
        IntVec2 const& GetDelta() const { return m_delta; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventMouseWheel>(m_delta);
        }

        EventMouseWheel(EventMouseWheel const&) = default;
        EventMouseWheel(EventMouseWheel&&) = default;
        EventMouseWheel& operator=(EventMouseWheel const&) = default;
        EventMouseWheel& operator=(EventMouseWheel&&) = default;
        ~EventMouseWheel() override {}

    private:
        IntVec2 m_delta;
    };
}
