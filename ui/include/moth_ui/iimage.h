#pragma once

#include "moth_ui/utils/vector.h"

namespace moth_ui {
    class IImage {
    public:
        virtual ~IImage() = default;

        virtual int GetWidth() const = 0;
        virtual int GetHeight() const = 0;
        virtual IntVec2 GetDimensions() const = 0;

        virtual void ImGui(moth_ui::IntVec2 const& size, moth_ui::FloatVec2 const& uv0 = { 0, 0 }, moth_ui::FloatVec2 const& uv1 = { 1, 1 }) const = 0;
    };
}
