#pragma once

namespace imgui_ext {
    bool InputString(char const* label, std::string* str);
    bool InputKeyframeValue(char const* label, ui::KeyframeValue* value);

    void InputIntVec2(char const* label, IntVec2* vec);
    void InputFloatVec2(char const* label, FloatVec2* vec);
}
