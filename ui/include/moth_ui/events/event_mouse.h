#pragma once

// Original header transitively provided event_cast via event.h — keep that.
#include "moth_ui/events/event.h"

// Moved to moth::core — re-exported here so existing moth_ui mouse events keep working.
#include <moth/core/event_mouse.h>

namespace moth_ui {
    using moth::core::MouseButton;
    using moth::core::EventMouseDown;
    using moth::core::EventMouseUp;
    using moth::core::EventMouseMove;
    using moth::core::EventMouseWheel;
}
