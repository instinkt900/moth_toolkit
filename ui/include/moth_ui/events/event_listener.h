#pragma once

#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    class EventListener {
    public:
        virtual bool OnEvent(Event const& event) = 0;

        EventListener() = default;
        EventListener(EventListener const&) = default;
        EventListener(EventListener&&) = default;
        EventListener& operator=(EventListener const&) = default;
        EventListener& operator=(EventListener&&) = default;
        virtual ~EventListener() = default;
    };
}
