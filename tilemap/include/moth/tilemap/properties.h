#pragma once

#include "moth/core/color.h"

#include <map>
#include <string>
#include <string_view>
#include <variant>

namespace moth::tilemap {
    /// @brief A typed Tiled property value.
    using PropertyValue = std::variant<bool, int, float, std::string, moth::core::Color>;

    /// @brief A named set of Tiled custom properties (name -> typed value).
    using Properties = std::map<std::string, PropertyValue>;

    /// @brief Returns the property @p name cast to @p T, or @p fallback if absent or a different type.
    template <typename T>
    T GetProperty(Properties const& props, std::string_view name, T const& fallback = T{}) {
        auto const it = props.find(std::string(name));
        if (it != props.end()) {
            if (auto const* value = std::get_if<T>(&it->second)) {
                return *value;
            }
        }
        return fallback;
    }

    /// @brief Returns @c true if a property named @p name exists.
    inline bool HasProperty(Properties const& props, std::string_view name) {
        return props.find(std::string(name)) != props.end();
    }
}
