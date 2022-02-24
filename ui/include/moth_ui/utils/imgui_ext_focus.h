#pragma once

#include "moth_ui/layout/layout_types.h"

namespace imgui_ext {
    struct FocusGroupContext {
        std::string currentFocusLabel;
    };
    
    void FocusGroupBegin(FocusGroupContext* context);
    void FocusGroupInputText(char const* label, std::string const& target, std::function<void(std::string const&)> const& onChanged, std::function<void()> const& onLostFocus);
    void FocusGroupInputInt(char const* label, int value, std::function<void(int)> const& onChanged, std::function<void()> const& onLostFocus);
    void FocusGroupInputFloat(char const* label, float value, std::function<void(float)> const& onChanged, std::function<void()> const& onLostFocus);
    void FocusGroupInputLayoutRect(char const* label, ui::LayoutRect value, std::function<void(ui::LayoutRect const&)> const& onChanged, std::function<void()> const& onLostFocus);
    void FocusGroupEnd();
}
