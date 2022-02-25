#pragma once

namespace moth_ui {
    template <typename T, typename U>
    inline T lerp(T const& a, T const& b, U const& factor) {
        return a + (b - a) * factor;
    }
}
