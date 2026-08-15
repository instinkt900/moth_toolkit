#pragma once

// Moved to moth::core — re-exported here so existing moth::gfx::EventEmitter keep working.
#include <moth/core/event_emitter.h>

namespace moth::gfx {
    using moth::core::LambdaHandle;
    using moth::core::LambdaWrapper;
    using moth::core::EventEmitter;
}
