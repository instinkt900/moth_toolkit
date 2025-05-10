#pragma once

#include <memory> 

namespace moth_ui {
    enum EventType : int {
        EVENTTYPE_KEY = 0,
        EVENTTYPE_MOUSE_DOWN = 1,
        EVENTTYPE_MOUSE_UP = 2,
        EVENTTYPE_MOUSE_MOVE = 3,
        EVENTTYPE_MOUSE_WHEEL = 4,
        EVENTTYPE_ANIMATION = 5,
        EVENTTYPE_ANIMATION_STARTED = 6,
        EVENTTYPE_ANIMATION_STOPPED = 7,

        EVENTTYPE_USER0 = 1000,
        EVENTTYPE_USER1 = 2000,
        EVENTTYPE_USER2 = 3000,
    };

    class Event {
    public:
        Event(int type)
            : m_type(type) {}

        Event(Event const&) = default;
        Event(Event&&) = default;
        Event& operator=(Event const&) = default;
        Event& operator=(Event&&) = default;
        virtual ~Event() = default;

        int GetType() const { return m_type; }
        virtual std::unique_ptr<Event> Clone() const = 0;

    protected:
        int m_type;
    };

    template <typename T>
    T const* event_cast(Event const& event) {
        if (event.GetType() == T::GetStaticType()) {
            return static_cast<T const*>(&event);
        }
        return nullptr;
    }
}
