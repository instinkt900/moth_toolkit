#pragma once

// Moved to moth::core — re-exported here so existing moth_graphics::EventEmitter keep working.
#include <moth/core/event_emitter.h>

namespace moth_graphics {
    using moth::core::LambdaHandle;
    using moth::core::LambdaWrapper;
    using moth::core::EventEmitter;
}
