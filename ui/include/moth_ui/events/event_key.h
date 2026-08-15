#pragma once

// Original header transitively provided event_cast via event.h — keep that.
#include "moth_ui/events/event.h"

// Moved to moth::core — re-exported here so existing moth_ui::EventKey keep working.
#include <moth/core/event_key.h>

namespace moth_ui {
    using moth::core::Key;
    using moth::core::KeyAction;
    using moth::core::EventKey;

    using moth::core::KeyMod_LeftShift;
    using moth::core::KeyMod_RightShift;
    using moth::core::KeyMod_LeftCtrl;
    using moth::core::KeyMod_RightCtrl;
    using moth::core::KeyMod_LeftAlt;
    using moth::core::KeyMod_RightAlt;
    using moth::core::KeyMod_Shift;
    using moth::core::KeyMod_Ctrl;
    using moth::core::KeyMod_Alt;
}
