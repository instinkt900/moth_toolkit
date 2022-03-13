#pragma once

#include <ostream>
#include <moth_ui/utils/color.h>

namespace moth_ui {
    std::ostream& operator<<(std::ostream& os, Color const& value) {
        os << "Color("
           << value.r << ", "
           << value.g << ", "
           << value.b << ", "
           << value.a << ")";
        return os;
    }
}
