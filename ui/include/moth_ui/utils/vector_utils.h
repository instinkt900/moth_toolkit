#pragma once

#include "moth_ui/utils/vector.h"
#include <cmath>

namespace moth_ui {
    template <typename T, int Dim>
    inline T Dot(Vector<T, Dim> const& a, Vector<T, Dim> const& b) {
        T sum = 0;
        for (int i = 0; i < Dim; ++i) {
            sum += a.data[i] * b.data[i];
        }
        return sum;
    }

    template <typename T, int Dim>
    inline T LengthSq(Vector<T, Dim> const& a) {
        return Dot(a, a);
    }

    template <typename T, int Dim>
    inline T Length(Vector<T, Dim> const& a) {
        return std::sqrt(LengthSq(a));
    }

    template <typename T, int Dim>
    inline T DistanceSq(Vector<T, Dim> const& a, Vector<T, Dim> const& b) {
        Vector<T, Dim> c = b - a;
        return LengthSq(c);
    }

    template <typename T, int Dim>
    inline T Distance(Vector<T, Dim> const& a, Vector<T, Dim> const& b) {
        Vector<T, Dim> c = b - a;
        return Length(c);
    }

    template <typename T, int Dim>
    inline Vector<T, Dim> Normalized(Vector<T, Dim> const& vec) {
        assert(Length(vec) > 0);
        return vec / Length(vec);
    }

    template <typename T>
    inline Vector<T, 2> Rotate2D(Vector<T, 2> const& vec, T angle) {
        Vector<T, 2> result;

        T cosTheta = std::cos(angle);
        T sinTheta = std::sin(angle);

        result.data[0] = cosTheta * vec.data[0] - sinTheta * vec.data[1];
        result.data[1] = sinTheta * vec.data[0] + cosTheta * vec.data[1];

        return result;
    }

    template <typename T, int Dim>
    inline Vector<T, Dim> Translate(Vector<T, Dim> const& vec, Vector<T, Dim> translation) {
        return vec + translation;
    }

    template <typename T>
    inline T Angle(Vector<T, 2> const& a, Vector<T, 2> const& b) {
        return std::atan2((a.x * b.y) - (a.y * b.x), (a.x * b.x) + (a.y * b.y));
    }
}
