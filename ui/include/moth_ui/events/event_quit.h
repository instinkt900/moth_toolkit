#pragma once

#include "event.h"

class EventQuit : public Event {
public:
    EventQuit()
        : Event(GetStaticType()) {}
    virtual ~EventQuit() {}

    static constexpr int GetStaticType() { return EVENTTYPE_QUIT; }

    std::unique_ptr<Event> Clone() const override {
        return std::make_unique<EventQuit>();
    }
};
