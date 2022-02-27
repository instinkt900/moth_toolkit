#pragma once

#include <ostream>
#include <moth_ui/color.h>

namespace moth_ui {
    std::ostream& operator<<(std::ostream& os, Color const& value) {
        os << "Color("
           << value.GetR() << ", "
           << value.GetG() << ", "
           << value.GetB() << ", "
           << value.GetA() << ")";
        return os;
    }
}
