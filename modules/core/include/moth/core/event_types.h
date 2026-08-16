#pragma once

#include <moth/core/event.h>

namespace moth::core {
    enum MothGraphicsEventType : int {
        EVENTTYPE_RENDERDEVICERESET = moth::core::EVENTTYPE_USER0,
        EVENTTYPE_RENDERTARGETRESET,
        EVENTTYPE_WINDOWSIZE,
        EVENTTYPE_REQUEST_QUIT,
        EVENTTYPE_QUIT,

        EVENTTYPE_GRAPHICSUSER0 = moth::core::EVENTTYPE_USER0 + 100
    };
}
