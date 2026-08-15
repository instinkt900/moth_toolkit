#pragma once

// Moved to moth::core — re-exported here so existing moth::ui::FloatMat4x4 keep working.
#include <moth/core/transform.h>

namespace moth::ui {
    using moth::core::FloatVec2;
    using moth::core::FloatMat4x4;
    using moth::core::kDefaultPivot;
    using moth::core::kDegToRad;
    using moth::core::kRadToDeg;
}
