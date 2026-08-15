#pragma once

// Moved to moth::core — re-exported here so existing moth_graphics window events keep working.
#include <moth/core/event_window.h>

namespace moth_graphics {
    using moth::core::EventRenderDeviceReset;
    using moth::core::EventRenderTargetReset;
    using moth::core::EventWindowSize;
    using moth::core::EventRequestQuit;
    using moth::core::EventQuit;
}
