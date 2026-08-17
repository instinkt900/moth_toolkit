#pragma once

// The logging facility now lives in moth::core. This header re-exports it into
// moth::ui for source compatibility (moth::ui::ILogger, moth::ui::log::*, ...).
#include <moth/core/log.h>

namespace moth::ui {
    using moth::core::LogLevel;
    using moth::core::ILogger;
    using moth::core::NullLogger;
    using moth::core::SetLogger;
    using moth::core::GetLogger;

    namespace log = moth::core::log;
}
