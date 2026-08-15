#pragma once

// Moved to moth::core — re-exported here so existing moth_ui::FloatMat4x4 keep working.
#include <moth/core/transform.h>

namespace moth_ui {
    using moth::core::FloatMat4x4;
    using moth::core::kDefaultPivot;
    using moth::core::kDegToRad;
    using moth::core::kRadToDeg;
}
