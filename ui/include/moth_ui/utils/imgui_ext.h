#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/utils/vector.h"

#define NLOHMANN_JSON_FROM_WITH_DEFAULT(v1) nlohmann_json_t.v1 = nlohmann_json_j.value(#v1, nlohmann_json_default_obj.v1);

#define NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Type, ...)                                                                                                                  \
    friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
    friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { Type nlohmann_json_default_obj; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__)) }

namespace imgui_ext {
    bool InputString(char const* label, std::string* str);
    bool InputKeyframeValue(char const* label, moth_ui::KeyframeValue* value);

    void InputIntVec2(char const* label, moth_ui::IntVec2* vec);
    void InputFloatVec2(char const* label, moth_ui::FloatVec2* vec);
}
