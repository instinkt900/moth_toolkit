#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/utils/vec2.h"

namespace imgui_ext {
    bool InputString(char const* label, std::string* str);
    bool InputKeyframeValue(char const* label, ui::KeyframeValue* value);

    void InputIntVec2(char const* label, IntVec2* vec);
    void InputFloatVec2(char const* label, FloatVec2* vec);
}
