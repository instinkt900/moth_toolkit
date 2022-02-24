#pragma once

#include "moth_ui/utils/smart_sdl.h"

namespace imgui_ext {
    inline void Inspect(char const* name, bool& value) {
        ImGui::Checkbox(name, &value);
    }

    inline void Inspect(char const* name, int& value) {
        ImGui::InputInt(name, &value);
    }

    inline void Inspect(char const* name, int const& value) {
        ImGui::LabelText(name, "%d", value);
    }

    inline void Inspect(char const* name, float& value) {
        ImGui::InputFloat(name, &value);
    }

    inline void Inspect(char const* name, char const* value) {
        ImGui::LabelText(name, "%s", value);
    }

    inline void Inspect(char const* name, std::string const& value) {
        ImGui::LabelText(name, "%s", value.c_str());
    }

    inline void Inspect(char const* name, SDL_Rect& value) {
        if (ImGui::CollapsingHeader(name)) {
            Inspect("x", value.x);
            Inspect("y", value.y);
            Inspect("w", value.w);
            Inspect("h", value.h);
        }
    }

    inline void Inspect(char const* name, TextureRef& value) {
        ImGui::Image(value.get(), ImVec2(100, 100));
    }

    template <typename T>
    void Inspect(char const* name, std::vector<T>& value) {
        if (ImGui::CollapsingHeader(name)) {
            for (int i = 0; i < value.size(); ++i) {
                auto const label = fmt::format("{}", i);
                Inspect(label.c_str(), value[i]);
            }
        }
    }

    template <typename T>
    void Inspect(char const* name, std::function<T> const& value) {
        ImGui::LabelText(name, "<function>");
    }

    template <typename T>
    void Inspect(char const* name, T* value) {
        if (nullptr == value) {
            ImGui::LabelText(name, "nullptr");
        } else {
            Inspect(name, *value);
        }
    }

    inline bool Inspect(char const* label, ui::LayoutRect& widgetBounds) {
        bool changed = false;
        ImGui::PushID(label);
        if (ImGui::CollapsingHeader(label)) {
            if (ImGui::TreeNode("Anchor")) {
                ImGui::PushItemWidth(50);
                changed |= ImGui::InputFloat("Top", &widgetBounds.anchor.topLeft.y, 0, 0, "%.2f");
                changed |= ImGui::InputFloat("Left", &widgetBounds.anchor.topLeft.x, 0, 0, "%.2f");
                changed |= ImGui::InputFloat("Bottom", &widgetBounds.anchor.bottomRight.y, 0, 0, "%.2f");
                changed |= ImGui::InputFloat("Right", &widgetBounds.anchor.bottomRight.x, 0, 0, "%.2f");
                ImGui::PopItemWidth();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Offset")) {
                ImGui::PushItemWidth(50);
                changed |= ImGui::InputFloat("Top", &widgetBounds.offset.topLeft.y, 0, 0, "%.2f");
                changed |= ImGui::InputFloat("Left", &widgetBounds.offset.topLeft.x, 0, 0, "%.2f");
                changed |= ImGui::InputFloat("Bottom", &widgetBounds.offset.bottomRight.y, 0, 0, "%.2f");
                changed |= ImGui::InputFloat("Right", &widgetBounds.offset.bottomRight.x, 0, 0, "%.2f");
                ImGui::PopItemWidth();
                ImGui::TreePop();
            }
        }
        ImGui::PopID();
        return changed;
    }

    inline bool Inspect(char const* label, IntRect& widgetRect) {
        bool changed = false;
        ImGui::PushID(label);
        if (ImGui::CollapsingHeader(label)) {
            ImGui::PushItemWidth(108);
            changed |= ImGui::InputInt("Top", &widgetRect.topLeft.y, 0);
            changed |= ImGui::InputInt("Left", &widgetRect.topLeft.x, 0);
            changed |= ImGui::InputInt("Bottom", &widgetRect.bottomRight.y, 0);
            changed |= ImGui::InputInt("Right", &widgetRect.bottomRight.x, 0);
            ImGui::PopItemWidth();
        }
        ImGui::PopID();
        return changed;
    }

}
