#pragma once

namespace moth_ui {
    template <typename T>
    struct Vec2 {
        T x{ 0 };
        T y{ 0 };

        template <typename U>
        explicit operator Vec2<U>() const {
            return Vec2<U>{ static_cast<U>(x), static_cast<U>(y) };
        }

        template <typename U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        Vec2<T>& operator+=(U const& other) {
            x = static_cast<T>(x + other);
            y = static_cast<T>(y + other);
            return *this;
        }

        Vec2<T>& operator+=(Vec2<T> const& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        template <typename U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        Vec2<T>& operator-=(U const& other) {
            x = static_cast<T>(x - other);
            y = static_cast<T>(y - other);
            return *this;
        }

        Vec2<T>& operator-=(Vec2<T> const& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        template <typename U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        Vec2<T>& operator*=(U const& other) {
            x = static_cast<T>(x * other);
            y = static_cast<T>(y * other);
            return *this;
        }

        template <typename U>
        Vec2<T>& operator*=(Vec2<U> const& other) {
            x = static_cast<T>(x * other.x);
            y = static_cast<T>(y * other.y);
            return *this;
        }

        template <typename U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        Vec2<T>& operator/=(U const& other) {
            x = static_cast<T>(x / other);
            y = static_cast<T>(y / other);
            return *this;
        }

        template <typename U>
        Vec2<T>& operator/=(Vec2<U> const& other) {
            x = static_cast<T>(x / other.x);
            y = static_cast<T>(y / other.y);
            return *this;
        }

        template <typename U>
        Vec2<T> operator+(U const& other) const {
            Vec2<T> ret = *this;
            ret += other;
            return ret;
        }

        template <typename U>
        Vec2<T> operator-(U const& other) const {
            Vec2<T> ret = *this;
            ret -= other;
            return ret;
        }

        template <typename U>
        Vec2<T> operator*(U const& other) const {
            Vec2<T> ret = *this;
            ret *= other;
            return ret;
        }

        template <typename U>
        Vec2<T> operator/(U const& other) const {
            Vec2<T> ret = *this;
            ret /= other;
            return ret;
        }
    };

    using IntVec2 = Vec2<int>;
    using FloatVec2 = Vec2<float>;

    template <typename T>
    inline bool operator==(Vec2<T> const& a, Vec2<T> const& b) {
        return (a.x == b.x) && (a.y == b.y);
    }

    template <typename T>
    inline bool operator!=(Vec2<T> const& a, Vec2<T> const& b) {
        return !(a == b);
    }

    template <typename T>
    inline void to_json(nlohmann::json& j, Vec2<T> const& vec) {
        j = nlohmann::json{ { "x", vec.x }, { "y", vec.y } };
    }

    template <typename T>
    inline void from_json(nlohmann::json const& j, Vec2<T>& vec) {
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
    }
}
