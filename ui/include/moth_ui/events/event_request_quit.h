#pragma once

#include "event.h"

namespace moth_ui {
    class EventRequestQuit : public Event {
    public:
        EventRequestQuit()
            : Event(GetStaticType()) {}
        virtual ~EventRequestQuit() {}

        static constexpr int GetStaticType() { return EVENTTYPE_REQUEST_QUIT; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventRequestQuit>();
        }
    };
}
