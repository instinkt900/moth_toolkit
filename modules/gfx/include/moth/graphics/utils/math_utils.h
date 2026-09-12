#pragma once

#include <moth/core/angle.h>
#include <moth/core/interp.h>

namespace moth::gfx {
    template<typename T, typename F>
    inline T Lerp(T const& min, T const& max, F const factor) {
        return min + (max - min) * factor;
    }

    using moth::core::DegToRad;
    using moth::core::RadToDeg;
    using moth::core::InterpType;
    using moth::core::Interp;
    using moth::core::F_PI;
}

