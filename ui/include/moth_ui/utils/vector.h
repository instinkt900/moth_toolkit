#pragma once

#include <array>

static_assert(true); // fixing clang-tidy bug for pragma pack. https://stackoverflow.com/questions/72456118/why-does-clang-give-a-warning-unterminated-pragma-pack-push-at-end-of-f
#pragma pack(push, 1)

//NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-constant-array-index,cppcoreguidelines-pro-type-member-init)

namespace moth_ui {
    /**
     * @brief Storage base for an N-dimensional vector of type T.
     *
     * Specialisations for 2, 3, and 4 dimensions expose named members
     * (x/y/z/w and r/g/b/a) via anonymous unions.
     *
     * @tparam T   Element type.
     * @tparam Dim Number of dimensions.
     */
    template <class T, int Dim>
    class VectorData {
    public:
        T data[Dim] = { 0 };
    };

    /// @brief Two-dimensional specialisation exposing @c x and @c y members.
    template <class T>
    class VectorData<T, 2> {
    public:
        union {
            struct {
                T x, y;
            };
            T data[2] = { 0 };
        };
    };

    /// @brief Three-dimensional specialisation exposing @c x/y/z and @c r/g/b members.
    template <class T>
    class VectorData<T, 3> {
    public:
        union {
            struct {
                T x, y, z;
            };
            struct {
                T r, g, b;
            };
            T data[3] = { 0 };
        };
    };

    /// @brief Four-dimensional specialisation exposing @c x/y/z/w and @c r/g/b/a members.
    template <class T>
    class VectorData<T, 4> {
    public:
        union {
            struct {
                T x, y, z, w;
            };
            struct {
                T r, g, b, a;
            };
            T data[4] = { 0 };
        };
    };

    /**
     * @brief Fixed-size mathematical vector with arithmetic operators.
     *
     * @tparam T   Element type (e.g. @c int, @c float).
     * @tparam Dim Number of dimensions.
     */
    template <class T, int Dim>
    class Vector : public VectorData<T, Dim> {
    public:
        using VectorData<T, Dim>::data;

        Vector() {}

        /**
         * @brief Constructs a vector from individual scalar arguments.
         *
         * Enabled only when the number of arguments matches @p Dim and all
         * arguments are convertible to @p T.
         */
        template <class... Scalars, std::enable_if_t<Dim >= 2 && std::conjunction_v<std::is_convertible<Scalars, T>...> && sizeof...(Scalars) == Dim, bool> = true>
        Vector(Scalars... scalars) {
            *reinterpret_cast<std::array<T, Dim>*>(data) = { T(scalars)... };
        }

        /**
         * @brief Explicit element-wise cast to a vector of a different type.
         * @tparam U Target element type.
         */
        template <typename U>
        explicit operator Vector<U, Dim>() const {
            Vector<U, Dim> result;
            for (int i = 0; i < Dim; ++i) {
                result.data[i] = static_cast<U>(data[i]);
            }
            return result;
        }

        /// @brief Adds @p other element-wise.
        Vector<T, Dim>& operator+=(Vector<T, Dim> const& other) {
            for (int i = 0; i < Dim; ++i) {
                data[i] += other.data[i];
            }
            return *this;
        }

        /// @brief Subtracts @p other element-wise.
        Vector<T, Dim>& operator-=(Vector<T, Dim> const& other) {
            for (int i = 0; i < Dim; ++i) {
                data[i] -= other.data[i];
            }
            return *this;
        }

        /// @brief Multiplies by @p other element-wise.
        Vector<T, Dim>& operator*=(Vector<T, Dim> const& other) {
            for (int i = 0; i < Dim; ++i) {
                data[i] *= other.data[i];
            }
            return *this;
        }

        /// @brief Divides by @p other element-wise.
        Vector<T, Dim>& operator/=(Vector<T, Dim> const& other) {
            for (int i = 0; i < Dim; ++i) {
                data[i] /= other.data[i];
            }
            return *this;
        }

        /// @brief Adds a scalar to every element.
        template <typename U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        Vector<T, Dim>& operator+=(U const& other) {
            for (int i = 0; i < Dim; ++i) {
                data[i] += other;
            }
            return *this;
        }

        /// @brief Subtracts a scalar from every element.
        template <typename U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        Vector<T, Dim>& operator-=(U const& other) {
            for (int i = 0; i < Dim; ++i) {
                data[i] -= other;
            }
            return *this;
        }

        /// @brief Multiplies every element by a scalar.
        template <typename U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        Vector<T, Dim>& operator*=(U const& other) {
            for (int i = 0; i < Dim; ++i) {
                data[i] *= other;
            }
            return *this;
        }

        /// @brief Divides every element by a scalar.
        template <typename U, std::enable_if_t<std::is_arithmetic<U>::value, bool> = true>
        Vector<T, Dim>& operator/=(U const& other) {
            for (int i = 0; i < Dim; ++i) {
                data[i] /= other;
            }
            return *this;
        }

        /// @brief Returns the element-wise sum with @p other.
        template <typename U>
        Vector<T, Dim> operator+(U const& other) const {
            auto ret = *this;
            ret += other;
            return ret;
        }

        /// @brief Returns the element-wise difference with @p other.
        template <typename U>
        Vector<T, Dim> operator-(U const& other) const {
            auto ret = *this;
            ret -= other;
            return ret;
        }

        /// @brief Returns the component-wise negation.
        Vector<T, Dim> operator-() const {
            auto ret = *this;
            for (int i = 0; i < Dim; ++i) {
                ret.data[i] = -ret.data[i];
            }
            return ret;
        }

        /// @brief Returns the element-wise product with @p other.
        template <typename U>
        Vector<T, Dim> operator*(U const& other) const {
            auto ret = *this;
            ret *= other;
            return ret;
        }

        /// @brief Returns the element-wise quotient with @p other.
        template <typename U>
        Vector<T, Dim> operator/(U const& other) const {
            auto ret = *this;
            ret /= other;
            return ret;
        }
    };

    /// @brief Returns @c true if all elements of @p a and @p b are equal.
    template <typename T, int Dim>
    inline bool operator==(Vector<T, Dim> const& a, Vector<T, Dim> const& b) {
        for (int i = 0; i < Dim; ++i) {
            if (a.data[i] != b.data[i]) {
                return false;
            }
        }
        return true;
    }

    /// @brief Returns @c true if any element of @p a differs from @p b.
    template <typename T, int Dim>
    inline bool operator!=(Vector<T, Dim> const& a, Vector<T, Dim> const& b) {
        return !(a == b);
    }

    /**
     * @brief Scalar-on-left multiplication: @p other * @p vec.
     * @param other Scalar factor.
     * @param vec   Vector to scale.
     */
    template <typename T, int Dim>
    inline Vector<T, Dim> operator*(T other, Vector<T, Dim> const& vec) {
        auto ret = vec;
        ret *= other;
        return ret;
    }

    /// @brief Alias for a 2D vector of @c float.
    using FloatVec2 = Vector<float, 2>;
    /// @brief Alias for a 2D vector of @c int.
    using IntVec2 = Vector<int, 2>;
}

//NOLINTEND(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-constant-array-index,cppcoreguidelines-pro-type-member-init)

#pragma pack(pop)
