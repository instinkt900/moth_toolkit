#pragma once

#include "moth_ui/events/event.h"
#include "moth_ui/utils/vector.h"

#include <memory>

namespace moth_ui {
    enum class MouseButton {
        Unknown,
        Left,
        Middle,
        Right,
    };

    class EventMouseDown : public Event {
    public:
        EventMouseDown(MouseButton button, IntVec2 const& position)
            : Event(GetStaticType())
            , m_button(button)
            , m_position(position) {}

        static constexpr int GetStaticType() { return EVENTTYPE_MOUSE_DOWN; }

        MouseButton GetButton() const { return m_button; }
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

    class EventMouseUp : public Event {
    public:
        EventMouseUp(MouseButton button, IntVec2 const& position)
            : Event(GetStaticType())
            , m_button(button)
            , m_position(position) {}

        static constexpr int GetStaticType() { return EVENTTYPE_MOUSE_UP; }

        MouseButton GetButton() const { return m_button; }
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

    class EventMouseMove : public Event {
    public:
        EventMouseMove(IntVec2 const& position, FloatVec2 const& delta)
            : Event(GetStaticType())
            , m_position(position)
            , m_delta(delta) {}

        static constexpr int GetStaticType() { return EVENTTYPE_MOUSE_MOVE; }

        IntVec2 const& GetPosition() const { return m_position; }
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

    class EventMouseWheel : public Event {
    public:
        EventMouseWheel(IntVec2 const& delta)
            : Event(GetStaticType())
            , m_delta(delta) {}

        static constexpr int GetStaticType() { return EVENTTYPE_MOUSE_WHEEL; }

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
