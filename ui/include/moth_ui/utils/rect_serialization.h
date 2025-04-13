#pragma once

#include "moth_ui/utils/rect.h"
#include "moth_ui/utils/vector_serialization.h"

namespace moth_ui {
    template <typename T>
    inline void to_json(nlohmann::json& j, Rect<T> const& rect) {
        j = nlohmann::json{ { "topLeft", rect.topLeft }, { "bottomRight", rect.bottomRight } };
    }

    template <typename T>
    inline void from_json(nlohmann::json const& j, Rect<T>& rect) {
        j.at("topLeft").get_to(rect.topLeft);
        j.at("bottomRight").get_to(rect.bottomRight);
    }
}
