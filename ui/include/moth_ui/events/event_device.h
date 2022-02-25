#pragma once

#include "event.h"

namespace moth_ui {
    class EventRenderDeviceReset : public Event {
    public:
        EventRenderDeviceReset()
            : Event(GetStaticType()) {}
        virtual ~EventRenderDeviceReset() {}

        static constexpr int GetStaticType() { return EVENTTYPE_RENDERDEVICERESET; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventRenderDeviceReset>();
        }
    };

    class EventRenderTargetReset : public Event {
    public:
        EventRenderTargetReset()
            : Event(GetStaticType()) {}
        virtual ~EventRenderTargetReset() {}

        static constexpr int GetStaticType() { return EVENTTYPE_RENDERTARGETRESET; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventRenderTargetReset>();
        }
    };
}
