#pragma once

// Moved to moth::core — re-exported here so existing moth::ui::Event keep working.
#include <moth/core/event.h>

namespace moth::ui {
    using moth::core::EventType;
    using moth::core::Event;
    using moth::core::event_cast;

    using moth::core::EVENTTYPE_KEY;
    using moth::core::EVENTTYPE_MOUSE_DOWN;
    using moth::core::EVENTTYPE_MOUSE_UP;
    using moth::core::EVENTTYPE_MOUSE_MOVE;
    using moth::core::EVENTTYPE_MOUSE_WHEEL;
    using moth::core::EVENTTYPE_ANIMATION;
    using moth::core::EVENTTYPE_ANIMATION_STARTED;
    using moth::core::EVENTTYPE_ANIMATION_STOPPED;
    using moth::core::EVENTTYPE_FLIPBOOK_STARTED;
    using moth::core::EVENTTYPE_FLIPBOOK_STOPPED;
    using moth::core::EVENTTYPE_USER0;
    using moth::core::EVENTTYPE_USER1;
    using moth::core::EVENTTYPE_USER2;
}
