#pragma once

// Moved to moth::core — re-exported here so existing moth::ui::Interp and the
// free easing functions keep working.
#include <moth/core/interp.h>

namespace moth::ui {
    using moth::core::InterpType;
    using moth::core::Interp;
    using moth::core::F_PI;
    using moth::core::InterpFunction;
    using moth::core::InterpFuncs;

    using moth::core::interpStep;
    using moth::core::interpLinear;
    using moth::core::interpSmooth;
    using moth::core::interpSineIn;
    using moth::core::interpSineOut;
    using moth::core::interpSineInOut;
    using moth::core::interpQuadIn;
    using moth::core::interpQuadOut;
    using moth::core::interpQuadInOut;
    using moth::core::interpCubicIn;
    using moth::core::interpCubicOut;
    using moth::core::interpCubicInOut;
    using moth::core::interpQuartIn;
    using moth::core::interpQuartOut;
    using moth::core::interpQuartInOut;
    using moth::core::interpQuintIn;
    using moth::core::interpQuintOut;
    using moth::core::interpQuintInOut;
    using moth::core::interpExpoIn;
    using moth::core::interpExpoOut;
    using moth::core::interpExpoInOut;
    using moth::core::interpCircIn;
    using moth::core::interpCircOut;
    using moth::core::interpCircInOut;
    using moth::core::interpBackIn;
    using moth::core::interpBackOut;
    using moth::core::interpBackInOut;
    using moth::core::interpElasticIn;
    using moth::core::interpElasticOut;
    using moth::core::interpElasticInOut;
    using moth::core::interpBounceIn;
    using moth::core::interpBounceOut;
    using moth::core::interpBounceInOut;
}
