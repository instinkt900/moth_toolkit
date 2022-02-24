#pragma once

#include "event.h"

class EventWindowSize : public Event {
public:
    EventWindowSize(int width, int height)
        : Event(GetStaticType())
        , m_width(width)
        , m_height(height) {}
    virtual ~EventWindowSize() {}

    static constexpr int GetStaticType() { return EVENTTYPE_WINDOWSIZE; }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    std::unique_ptr<Event> Clone() const override {
        return std::make_unique<EventWindowSize>(m_width, m_height);
    }

private:
    int m_width = 0;
    int m_height = 0;
};
