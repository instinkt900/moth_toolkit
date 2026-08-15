#pragma once

// Moved to moth::core — re-exported here so existing moth_graphics event types keep working.
#include <moth/core/event_types.h>

namespace moth_graphics {
    using moth::core::MothGraphicsEventType;
    using moth::core::EVENTTYPE_RENDERDEVICERESET;
    using moth::core::EVENTTYPE_RENDERTARGETRESET;
    using moth::core::EVENTTYPE_WINDOWSIZE;
    using moth::core::EVENTTYPE_REQUEST_QUIT;
    using moth::core::EVENTTYPE_QUIT;
    using moth::core::EVENTTYPE_GRAPHICSUSER0;
}
