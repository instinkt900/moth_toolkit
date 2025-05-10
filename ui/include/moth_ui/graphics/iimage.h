#pragma once

#include "moth_ui/utils/vector.h"

namespace moth_ui {
    class IImage {
    public:
        virtual int GetWidth() const = 0;
        virtual int GetHeight() const = 0;
        virtual IntVec2 GetDimensions() const = 0;

        virtual void ImGui(moth_ui::IntVec2 const& size, moth_ui::FloatVec2 const& uv0 = { 0, 0 }, moth_ui::FloatVec2 const& uv1 = { 1, 1 }) const = 0;

        IImage() = default;
        IImage(IImage const&) = default;
        IImage(IImage&&) = default;
        IImage& operator=(IImage const&) = default;
        IImage& operator=(IImage&&) = default;
        virtual ~IImage() = default;
    };
}
