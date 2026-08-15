#pragma once

#include <algorithm>
#include <cmath>

namespace moth_graphics::graphics::detail {
    // Segment count for a filled-circle triangle fan. Roughly 2px chord error;
    // clamped so tiny circles still look round and very large ones don't blow
    // up the vertex count.
    inline int CircleSegmentCount(float radius) {
        // Call sites only reject radius <= 0, which lets NaN/Inf through; casting
        // a non-finite ceil() result to int is undefined behaviour. Fall back to
        // the minimum segment count for those.
        if (!std::isfinite(radius)) {
            return 12;
        }
        int const n = static_cast<int>(std::ceil(radius * 1.5f));
        return std::clamp(n, 12, 64);
    }
}
