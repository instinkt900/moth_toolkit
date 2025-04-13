#pragma once

#include <memory> 

namespace moth_ui {
    enum EventType : int {
        EVENTTYPE_KEY,
        EVENTTYPE_MOUSE_DOWN,
        EVENTTYPE_MOUSE_UP,
        EVENTTYPE_MOUSE_MOVE,
        EVENTTYPE_MOUSE_WHEEL,
        EVENTTYPE_ANIMATION,
        EVENTTYPE_ANIMATION_STARTED,
        EVENTTYPE_ANIMATION_STOPPED,

        EVENTTYPE_USER0 = 1000,
        EVENTTYPE_USER1 = 2000,
        EVENTTYPE_USER2 = 3000,
    };

    class Event {
    public:
        Event(int type)
            : m_type(type) {}
        virtual ~Event() {}

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
