#pragma once

#include <ostream>
#include <moth_ui/utils/vector.h>

namespace moth_ui {
    template <typename T, int Dim>
    std::ostream& operator<<(std::ostream& os, Vector<T, Dim> const& vector) {
        os << "Vector(";
        for (int i = 0; i < Dim; ++i) {
            if (i > 0)
                os << ", ";
            os << vector.data[i];
        }
        os << ")";
        return os;
    }
}
