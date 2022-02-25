#pragma once

#include "ui_fwd.h"

namespace moth_ui {
    class EventListener {
    public:
        virtual bool OnEvent(Event const& event) = 0;
    };
}
