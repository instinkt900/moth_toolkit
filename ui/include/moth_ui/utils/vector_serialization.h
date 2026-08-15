#pragma once

#include "moth_ui/utils/vector.h"
#include <nlohmann/json.hpp>

namespace moth_ui {
    /**
     * @brief Serialises a Vector to a JSON array.
     * @tparam T   Element type.
     * @tparam Dim Number of dimensions.
     * @param j   Output JSON value.
     * @param vec Vector to serialise.
     */
    template <typename T, int Dim>
    inline void to_json(nlohmann::json& j, Vector<T, Dim> const& vec) {
        j = nlohmann::json::array();
        for (int i = 0; i < Dim; ++i) {
            j.push_back(vec.data[i]);
        }
    }

    /**
     * @brief Deserialises a Vector from a JSON array or object.
     *
     * Accepts both array form @c [x,y,...] and (for backward compatibility)
     * object form @c {"x":…,"y":…} for vectors up to dimension 4.
     *
     * @tparam T   Element type.
     * @tparam Dim Number of dimensions.
     * @param j   Source JSON value.
     * @param vec Vector to populate.
     */
    template <typename T, int Dim>
    inline void from_json(nlohmann::json const& j, Vector<T, Dim>& vec) {
        // Preferred: array form [x, y, (z, w...)]
        if (j.is_array()) {
            if (j.size() != static_cast<std::size_t>(Dim)) {
                throw std::runtime_error("Vector: expected JSON array of size " + std::to_string(Dim));
            }
            for (int i = 0; i < Dim; ++i) {
                j.at(static_cast<std::size_t>(i)).get_to(vec.data[i]); // bounds-checked
            }
            return;
        }
        // Backward-compat: accept object form with x/y(/z/w) keys for up to 4D
        if (j.is_object() && Dim <= 4) {
            static constexpr const std::array keys = { "x", "y", "z", "w" };
            for (int i = 0; i < Dim; ++i) {
                j.at(keys.at(i)).get_to(vec.data[i]); // throws if key missing
            }
            return;
        }
        throw std::runtime_error("Vector: expected JSON array or {x,y[,z[,w]]} object");
    }
}
