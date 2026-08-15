#pragma once

#include "moth_ui/utils/vector.h"

namespace moth_ui {
    /**
     * @brief Axis-aligned rectangle defined by its top-left and bottom-right corners.
     *
     * @tparam T Element type (e.g. @c int, @c float).
     */
    template <typename T>
    struct Rect {
        Vector<T, 2> topLeft;     ///< Top-left corner of the rectangle.
        Vector<T, 2> bottomRight; ///< Bottom-right corner of the rectangle.

        Rect()
            : topLeft{}
            , bottomRight{} {}

        /**
         * @brief Constructs a rectangle from explicit corner points.
         * @param tl Top-left corner.
         * @param br Bottom-right corner.
         */
        Rect(Vector<T, 2> const& tl, Vector<T, 2> const& br)
            : topLeft(tl)
            , bottomRight(br) {}

        /// @brief Returns the x-coordinate of the left edge.
        T x() const { return topLeft.x; }
        /// @brief Returns the y-coordinate of the top edge.
        T y() const { return topLeft.y; }
        /// @brief Returns the width of the rectangle.
        T w() const { return bottomRight.x - topLeft.x; }
        /// @brief Returns the height of the rectangle.
        T h() const { return bottomRight.y - topLeft.y; }

        /// @brief Alias for @c x(). Returns the x-coordinate of the left edge.
        T left() const { return topLeft.x; }
        /// @brief Returns the x-coordinate of the right edge.
        T right() const { return bottomRight.x; }
        /// @brief Alias for @c y(). Returns the y-coordinate of the top edge.
        T top() const { return topLeft.y; }
        /// @brief Returns the y-coordinate of the bottom edge.
        T bottom() const { return bottomRight.y; }

        /// @brief Returns the width and height as a 2D vector.
        Vector<T, 2> dimensions() const { return bottomRight - topLeft; }

        /**
         * @brief Explicit element-wise cast to a rectangle of a different type.
         * @tparam U Target element type.
         */
        template <typename U>
        explicit operator Rect<U>() const {
            Rect<U> result;
            result.topLeft = static_cast<Vector<U, 2>>(topLeft);
            result.bottomRight = static_cast<Vector<U, 2>>(bottomRight);
            return result;
        }
    };

    /**
     * @brief Creates a Rect from position and size components.
     * @param x Left edge.
     * @param y Top edge.
     * @param w Width.
     * @param h Height.
     */
    template <typename T>
    Rect<T> MakeRect(T x, T y, T w, T h) {
        return { { x, y }, { x + w, y + h } };
    }

    /// @brief Alias for a rectangle with @c int coordinates.
    using IntRect = Rect<int>;
    /// @brief Alias for a rectangle with @c float coordinates.
    using FloatRect = Rect<float>;

    /// @brief Returns @c true if @p a and @p b have identical corners.
    template <typename T>
    inline bool operator==(Rect<T> const& a, Rect<T> const& b) {
        return (a.topLeft == b.topLeft) && (a.bottomRight == b.bottomRight);
    }

    /// @brief Returns @c true if @p a and @p b differ in any corner.
    template <typename T>
    inline bool operator!=(Rect<T> const& a, Rect<T> const& b) {
        return !(a == b);
    }

    /// @brief Adds the corners of @p b to @p a in place.
    template <typename T>
    inline Rect<T>& operator+=(Rect<T>& a, Rect<T> const& b) {
        a.topLeft += b.topLeft;
        a.bottomRight += b.bottomRight;
        return a;
    }

    /// @brief Subtracts the corners of @p b from @p a in place.
    template <typename T>
    inline Rect<T>& operator-=(Rect<T>& a, Rect<T> const& b) {
        a.topLeft -= b.topLeft;
        a.bottomRight -= b.bottomRight;
        return a;
    }

    /// @brief Translates @p a by @p b in place.
    template <typename T>
    inline Rect<T>& operator+=(Rect<T>& a, Vector<T, 2> const& b) {
        a.topLeft += b;
        a.bottomRight += b;
        return a;
    }

    /// @brief Translates @p a by @c -b in place.
    template <typename T>
    inline Rect<T>& operator-=(Rect<T>& a, Vector<T, 2> const& b) {
        a.topLeft -= b;
        a.bottomRight -= b;
        return a;
    }

    /// @brief Scales all corners of @p a by scalar @p b in place.
    template <typename T>
    inline Rect<T>& operator*=(Rect<T>& a, T b) {
        a.topLeft *= b;
        a.bottomRight *= b;
        return a;
    }

    /// @brief Divides all corners of @p a by scalar @p b in place.
    template <typename T>
    inline Rect<T>& operator/=(Rect<T>& a, T b) {
        a.topLeft /= b;
        a.bottomRight /= b;
        return a;
    }

    /// @brief Scales all corners of @p a element-wise by @p b in place.
    template <typename T>
    inline Rect<T>& operator*=(Rect<T>& a, Vector<T, 2> const& b) {
        a.topLeft *= b;
        a.bottomRight *= b;
        return a;
    }

    /// @brief Divides all corners of @p a element-wise by @p b in place.
    template <typename T>
    inline Rect<T>& operator/=(Rect<T>& a, Vector<T, 2> const& b) {
        a.topLeft /= b;
        a.bottomRight /= b;
        return a;
    }

    /// @brief Offsets all corners of @p a by scalar @p b in place.
    template <typename T>
    inline Rect<T>& operator+=(Rect<T>& a, T b) {
        a.topLeft.x += b;
        a.topLeft.y += b;
        a.bottomRight.x += b;
        a.bottomRight.y += b;
        return a;
    }

    /// @brief Offsets all corners of @p a by @c -b in place.
    template <typename T>
    inline Rect<T>& operator-=(Rect<T>& a, T b) {
        a.topLeft.x -= b;
        a.topLeft.y -= b;
        a.bottomRight.x -= b;
        a.bottomRight.y -= b;
        return a;
    }

    /// @brief Returns @p rect offset by scalar @p other added to every corner.
    template <typename T>
    inline Rect<T> operator+(Rect<T> const& rect, T other) {
        auto ret = rect;
        ret += other;
        return ret;
    }

    /// @brief Returns @p rect offset by scalar @p other subtracted from every corner.
    template <typename T>
    inline Rect<T> operator-(Rect<T> const& rect, T other) {
        auto ret = rect;
        ret -= other;
        return ret;
    }

    /// @brief Returns @p rect with every corner scaled by @p other.
    template <typename T>
    inline Rect<T> operator*(Rect<T> const& rect, T other) {
        auto ret = rect;
        ret *= other;
        return ret;
    }

    /// @brief Returns @p rect with every corner divided by @p other.
    template <typename T>
    inline Rect<T> operator/(Rect<T> const& rect, T other) {
        auto ret = rect;
        ret /= other;
        return ret;
    }

    /// @brief Returns @p rect translated by vector @p other.
    template <typename T>
    inline Rect<T> operator+(Rect<T> const& rect, Vector<T, 2> const& other) {
        auto ret = rect;
        ret += other;
        return ret;
    }

    /// @brief Returns @p rect translated by @c -other.
    template <typename T>
    inline Rect<T> operator-(Rect<T> const& rect, Vector<T, 2> const& other) {
        auto ret = rect;
        ret -= other;
        return ret;
    }

    /// @brief Returns @p rect with corners scaled element-wise by @p other.
    template <typename T>
    inline Rect<T> operator*(Rect<T> const& rect, Vector<T, 2> const& other) {
        auto ret = rect;
        ret *= other;
        return ret;
    }

    /// @brief Returns @p rect with corners divided element-wise by @p other.
    template <typename T>
    inline Rect<T> operator/(Rect<T> const& rect, Vector<T, 2> const& other) {
        auto ret = rect;
        ret /= other;
        return ret;
    }

    /**
     * @brief Returns @c true if all four coordinates of @p rect are zero.
     * @param rect Rectangle to test.
     */
    template <typename T>
    inline bool IsZero(Rect<T> const& rect) {
        return rect.topLeft.x == 0 && rect.topLeft.y == 0 && rect.bottomRight.x == 0 && rect.bottomRight.y == 0;
    }

    /**
     * @brief Returns @c true if @p point lies within @p rect (inclusive).
     * @param point Point to test.
     * @param rect  Rectangle to test against.
     */
    template <typename T, typename U>
    inline bool IsInRect(Vector<T, 2> const& point, Rect<U> const& rect) {
        return !(point.x > rect.bottomRight.x || point.x < rect.topLeft.x ||
                 point.y > rect.bottomRight.y || point.y < rect.topLeft.y);
    }

    /**
     * @brief Returns @c true if @p rect1 and @p rect2 overlap.
     * @param rect1 First rectangle.
     * @param rect2 Second rectangle.
     */
    template <typename T>
    inline bool Intersects(Rect<T> const& rect1, Rect<T> const& rect2) {
        return !(rect2.topLeft.x > rect1.bottomRight.x || rect2.bottomRight.x < rect1.topLeft.x || rect2.topLeft.y > rect1.bottomRight.y || rect2.bottomRight.y < rect1.topLeft.y);
    }
}
