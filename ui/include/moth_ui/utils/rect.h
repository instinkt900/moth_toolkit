#pragma once

#include "vector.h"

namespace moth_ui {
    template <typename T>
    struct Rect {
        Vector<T, 2> topLeft;
        Vector<T, 2> bottomRight;
    };

    using IntRect = Rect<int>;
    using FloatRect = Rect<float>;

    template <typename T>
    inline bool operator==(Rect<T> const& a, Rect<T> const& b) {
        return (a.topLeft == b.topLeft) && (a.bottomRight == b.bottomRight);
    }

    template <typename T>
    inline bool operator!=(Rect<T> const& a, Rect<T> const& b) {
        return !(a == b);
    }

    template <typename T>
    inline Rect<T>& operator+=(Rect<T>& a, Rect<T> const& b) {
        a.topLeft += b.topLeft;
        a.bottomRight += b.bottomRight;
        return a;
    }

    template <typename T>
    inline Rect<T>& operator-=(Rect<T>& a, Rect<T> const& b) {
        a.topLeft -= b.topLeft;
        a.bottomRight -= b.bottomRight;
        return a;
    }

    template <typename T>
    inline bool IsZero(Rect<T> const& rect) {
        return rect.topLeft.x == 0 && rect.topLeft.y == 0 && rect.bottomRight.x == 0 && rect.bottomRight.y == 0;
    }

    template <typename T, typename U>
    inline bool IsInRect(Vector<T, 2> const& point, Rect<U> const& rect) {
        if (point.x > rect.bottomRight.x || point.x < rect.topLeft.x ||
            point.y > rect.bottomRight.y || point.y < rect.topLeft.y) {
            return false;
        }
        return true;
    }
}
