#pragma once

#include <algorithm>
#include <cmath>

namespace moth_graphics::graphics::detail {
    // Segment count for a filled-circle triangle fan. Roughly 2px chord error;
    // clamped so tiny circles still look round and very large ones don't blow
    // up the vertex count.
    inline int CircleSegmentCount(float radius) {
        int const n = static_cast<int>(std::ceil(radius * 1.5f));
        return std::clamp(n, 12, 64);
    }
}
