#pragma once

#include "moth_ui/utils/vector.h"
#include <algorithm>

namespace moth_ui {
    using Color = Vector<float, 4>;

    namespace BasicColors {
        static Color const Red{ 1.0f, 0.0f, 0.0f, 1.0f };
        static Color const Green{ 0.0f, 1.0f, 0.0f, 1.0f };
        static Color const Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
        static Color const White{ 1.0f, 1.0f, 1.0f, 1.0f };
        static Color const Black{ 0.0f, 0.0f, 0.0f, 1.0f };
    }

    inline Color Normalize(Color const& color) {
        return Color{ std::clamp(color.r, 0.0f, 1.0f), std::clamp(color.g, 0.0f, 1.0f), std::clamp(color.b, 0.0f, 1.0f), std::clamp(color.a, 0.0f, 1.0f) };
    }

    inline Color FromARGB(uint32_t argb) {
        uint8_t const a = (argb & 0xFF000000) >> 24;
        uint8_t const r = (argb & 0x00FF0000) >> 16;
        uint8_t const g = (argb & 0x0000FF00) >> 8;
        uint8_t const b = (argb & 0x000000FF);
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    inline Color FromRGBA(uint32_t rgba) {
        uint8_t const r = (rgba & 0xFF000000) >> 24;
        uint8_t const g = (rgba & 0x00FF0000) >> 16;
        uint8_t const b = (rgba & 0x0000FF00) >> 8;
        uint8_t const a = (rgba & 0x000000FF);
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    inline uint32_t ToRGBA(Color const& color) {
        uint32_t const r = static_cast<uint32_t>(std::round(color.r * 255));
        uint32_t const g = static_cast<uint32_t>(std::round(color.g * 255));
        uint32_t const b = static_cast<uint32_t>(std::round(color.b * 255));
        uint32_t const a = static_cast<uint32_t>(std::round(color.a * 255));
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    inline uint32_t ToARGB(Color const& color) {
        uint32_t const r = static_cast<uint32_t>(std::round(color.r * 255));
        uint32_t const g = static_cast<uint32_t>(std::round(color.g * 255));
        uint32_t const b = static_cast<uint32_t>(std::round(color.b * 255));
        uint32_t const a = static_cast<uint32_t>(std::round(color.a * 255));
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
}
