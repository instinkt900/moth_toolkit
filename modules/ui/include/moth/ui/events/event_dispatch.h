#pragma once

// Original header transitively provided event_cast via event.h — keep that.
#include "moth/ui/events/event.h"
#include "moth/ui/events/event_listener.h"

// Moved to moth::core — re-exported here so existing moth::ui::EventDispatch keep working.
#include <moth/core/event_dispatch.h>

namespace moth::ui {
    using moth::core::EventDispatch;
}
