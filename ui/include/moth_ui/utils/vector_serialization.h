#pragma once

#include "vector.h"

namespace moth_ui {
    template <typename T>
    inline void to_json(nlohmann::json& j, Vector<T, 2> const& vec) {
        j = nlohmann::json{ { "x", vec.x }, { "y", vec.y } };
    }

    template <typename T>
    inline void from_json(nlohmann::json const& j, Vector<T, 2>& vec) {
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
    }
}
