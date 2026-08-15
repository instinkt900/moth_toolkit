#pragma once

// Moved to moth::core — re-exported here so existing moth_ui::Rect keep working.
#include <moth/core/rect.h>

namespace moth_ui {
    using moth::core::Rect;
    using moth::core::IntRect;
    using moth::core::FloatRect;
    using moth::core::MakeRect;
    using moth::core::IsZero;
    using moth::core::IsInRect;
    using moth::core::Intersects;
}
