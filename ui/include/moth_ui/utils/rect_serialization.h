#pragma once

#include "moth_ui/utils/rect.h"
#include "moth_ui/utils/vector_serialization.h"

namespace moth_ui {
    /**
     * @brief Serialises a Rect to JSON as @c {"topLeft":…, "bottomRight":…}.
     * @tparam T Element type of the rectangle.
     * @param j    Output JSON value.
     * @param rect Rectangle to serialise.
     */
    template <typename T>
    inline void to_json(nlohmann::json& j, Rect<T> const& rect) {
        j = nlohmann::json{ { "topLeft", rect.topLeft }, { "bottomRight", rect.bottomRight } };
    }

    /**
     * @brief Deserialises a Rect from JSON.
     * @tparam T Element type of the rectangle.
     * @param j    Source JSON value.
     * @param rect Rectangle to populate.
     */
    template <typename T>
    inline void from_json(nlohmann::json const& j, Rect<T>& rect) {
        j.at("topLeft").get_to(rect.topLeft);
        j.at("bottomRight").get_to(rect.bottomRight);
    }
}
