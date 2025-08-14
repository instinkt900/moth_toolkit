#pragma once

#include "moth_ui/utils/vector.h"
#include <nlohmann/json.hpp>

namespace moth_ui {
    template <typename T, int Dim>
    inline void to_json(nlohmann::json& j, Vector<T, Dim> const& vec) {
        j = nlohmann::json::array();
        for (int i = 0; i < Dim; ++i) {
            j.push_back(vec.data[i]);
        }
    }

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
            static constexpr const char* keys[4] = { "x", "y", "z", "w" };
            for (int i = 0; i < Dim; ++i) {
                j.at(keys[i]).get_to(vec.data[i]); // throws if key missing
            }
            return;
        }
        throw std::runtime_error("Vector: expected JSON array or {x,y[,z[,w]]} object");
    }
}
