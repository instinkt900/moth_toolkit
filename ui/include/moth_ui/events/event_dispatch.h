#pragma once

// Original header transitively provided event_cast via event.h — keep that.
#include "moth_ui/events/event.h"
#include "moth_ui/events/event_listener.h"

// Moved to moth::core — re-exported here so existing moth_ui::EventDispatch keep working.
#include <moth/core/event_dispatch.h>

namespace moth_ui {
    using moth::core::EventDispatch;
}
