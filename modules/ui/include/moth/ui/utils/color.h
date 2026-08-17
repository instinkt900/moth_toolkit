#pragma once

// Moved to moth::core — re-exported here so existing moth::ui::Color keep working.
#include <moth/core/color.h>

namespace moth::ui {
    using moth::core::Color;
    using moth::core::Normalize;
    using moth::core::Limit;
    using moth::core::Clamp;
    using moth::core::FromARGB;
    using moth::core::FromRGBA;
    using moth::core::ToRGBA;
    using moth::core::ToARGB;
    using moth::core::ToABGR;
    using moth::core::Blend;
    namespace BasicColors = moth::core::BasicColors;
}
