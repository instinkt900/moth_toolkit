#pragma once

#include "moth_ui/moth_ui_fwd.h"

namespace moth_ui {
    /**
     * @brief Interface for objects that can receive UI events.
     *
     * Implement OnEvent() to process incoming events.  Return @c true to
     * indicate the event has been consumed and should not propagate further.
     */
    class IEventListener {
    public:
        /**
         * @brief Called when an event is dispatched to this listener.
         * @param event The event to process.
         * @return @c true if the event was handled (stops further propagation).
         */
        virtual bool OnEvent(Event const& event) = 0;

        IEventListener() = default;
        IEventListener(IEventListener const&) = default;
        IEventListener(IEventListener&&) = default;
        IEventListener& operator=(IEventListener const&) = default;
        IEventListener& operator=(IEventListener&&) = default;
        virtual ~IEventListener() = default;
    };
}
