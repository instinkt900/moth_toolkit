#pragma once

#include "moth_ui/graphics/blend_mode.h"
#include "moth_ui/utils/vector.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

namespace moth_ui {
    /**
     * @brief RGBA colour represented as four @c float components in the range [0, 1].
     *
     * @c Color inherits the full arithmetic of @c Vector (operator+, operator*, etc.).
     * These operate per-component and are @em not alpha blending. For proper colour
     * compositing, use the free function @c Blend() with an explicit @c BlendMode.
     */
    using Color = Vector<float, 4>;

    /// @brief Predefined basic colours for convenience.
    namespace BasicColors {
        inline const Color Red{ 1.0f, 0.0f, 0.0f, 1.0f };
        inline const Color Green{ 0.0f, 1.0f, 0.0f, 1.0f };
        inline const Color Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
        inline const Color White{ 1.0f, 1.0f, 1.0f, 1.0f };
        inline const Color Black{ 0.0f, 0.0f, 0.0f, 1.0f };
    }

    /**
     * @brief Scales all components so the largest equals 1.
     * @param color Input colour.
     * @return Normalised colour with the same hue/ratio but maximum component == 1.
     */
    inline Color Normalize(Color const& color) {
        auto const maxComponent = std::max({ color.r, color.g, color.b, color.a });
        if (maxComponent <= 0.0f) {
            return color;
        }
        auto const factor = 1.0f / maxComponent;
        return color * factor;
    }

    /**
     * @brief Like Normalize, but only scales down when the largest component exceeds 1.
     * @param color Input colour.
     * @return Colour with every component ≤ 1, unchanged if already in range.
     */
    inline Color Limit(Color const& color) {
        auto const maxComponent = std::max({ color.r, color.g, color.b, color.a });
        if (maxComponent > 1.0f) {
            return Normalize(color);
        }
        return color;
    }

    /**
     * @brief Clamps every component of @p color to [0, 1].
     * @param color Input colour.
     * @return Colour with all components clamped.
     */
    inline Color Clamp(Color const& color) {
        return Color{ std::clamp(color.r, 0.0f, 1.0f), std::clamp(color.g, 0.0f, 1.0f), std::clamp(color.b, 0.0f, 1.0f), std::clamp(color.a, 0.0f, 1.0f) };
    }

    /**
     * @brief Converts a packed ARGB 32-bit integer to a floating-point Color.
     * @param argb Packed colour as 0xAARRGGBB.
     */
    inline Color FromARGB(uint32_t argb) {
        auto const a = static_cast<float>((argb & 0xFF000000) >> 24);
        auto const r = static_cast<float>((argb & 0x00FF0000) >> 16);
        auto const g = static_cast<float>((argb & 0x0000FF00) >> 8);
        auto const b = static_cast<float>(argb & 0x000000FF);
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    /**
     * @brief Converts a packed RGBA 32-bit integer to a floating-point Color.
     * @param rgba Packed colour as 0xRRGGBBAA.
     */
    inline Color FromRGBA(uint32_t rgba) {
        auto const r = static_cast<float>((rgba & 0xFF000000) >> 24);
        auto const g = static_cast<float>((rgba & 0x00FF0000) >> 16);
        auto const b = static_cast<float>((rgba & 0x0000FF00) >> 8);
        auto const a = static_cast<float>(rgba & 0x000000FF);
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    /**
     * @brief Converts a floating-point Color to a packed RGBA 32-bit integer.
     * @param color Input colour (components are clamped before conversion).
     * @return Packed colour as 0xRRGGBBAA.
     */
    inline uint32_t ToRGBA(Color const& color) {
        auto const limitedColor = Clamp(color);
        uint32_t const r = static_cast<uint32_t>(std::round(limitedColor.r * 255));
        uint32_t const g = static_cast<uint32_t>(std::round(limitedColor.g * 255));
        uint32_t const b = static_cast<uint32_t>(std::round(limitedColor.b * 255));
        uint32_t const a = static_cast<uint32_t>(std::round(limitedColor.a * 255));
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    /**
     * @brief Converts a floating-point Color to a packed ARGB 32-bit integer.
     * @param color Input colour (components are clamped before conversion).
     * @return Packed colour as 0xAARRGGBB.
     */
    inline uint32_t ToARGB(Color const& color) {
        auto const limitedColor = Clamp(color);
        uint32_t const r = static_cast<uint32_t>(std::round(limitedColor.r * 255));
        uint32_t const g = static_cast<uint32_t>(std::round(limitedColor.g * 255));
        uint32_t const b = static_cast<uint32_t>(std::round(limitedColor.b * 255));
        uint32_t const a = static_cast<uint32_t>(std::round(limitedColor.a * 255));
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    /**
     * @brief Converts a floating-point Color to a packed ABGR 32-bit integer.
     * @param color Input colour (components are clamped before conversion).
     * @return Packed colour as 0xAABBGGRR.
     */
    inline uint32_t ToABGR(Color const& color) {
        auto const limitedColor = Clamp(color);
        uint32_t const r = static_cast<uint32_t>(std::round(limitedColor.r * 255));
        uint32_t const g = static_cast<uint32_t>(std::round(limitedColor.g * 255));
        uint32_t const b = static_cast<uint32_t>(std::round(limitedColor.b * 255));
        uint32_t const a = static_cast<uint32_t>(std::round(limitedColor.a * 255));
        return (a << 24) | (b << 16) | (g << 8) | r;
    }

    /**
     * @brief Blends a source colour over a destination colour using a given blend mode.
     * @param srcColor Source (foreground) colour.
     * @param dstColor Destination (background) colour.
     * @param blend    Blend equation to apply.
     * @return The composited result colour.
     */
    inline Color Blend(Color const& srcColor, Color const& dstColor, BlendMode blend) {
        moth_ui::Color srcColorFactor;
        moth_ui::Color dstColorFactor;
        float srcAlphaFactor = NAN;
        float dstAlphaFactor = NAN;
        switch (blend) {
        default:
        case moth_ui::BlendMode::Replace:
            srcColorFactor = { 1.0f, 1.0f, 1.0f, 0.0f };
            dstColorFactor = { 0.0f, 0.0f, 0.0f, 0.0f };
            srcAlphaFactor = 1.0f;
            dstAlphaFactor = 0.0f;
            break;
        case moth_ui::BlendMode::Add:
            srcColorFactor = { srcColor.a, srcColor.a, srcColor.a, 0.0f };
            dstColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
            srcAlphaFactor = 0.0f;
            dstAlphaFactor = 1.0f;
            break;
        case moth_ui::BlendMode::Alpha:
            srcColorFactor = { srcColor.a, srcColor.a, srcColor.a, 0 };
            dstColorFactor = { 1.0f - srcColor.a, 1.0f - srcColor.a, 1.0f - srcColor.a, 0.0f };
            srcAlphaFactor = 1.0f;
            dstAlphaFactor = 1.0f - srcColor.a;
            break;
        case moth_ui::BlendMode::Modulate:
            srcColorFactor = { 0.0f, 0.0f, 0.0f, 0.0f };
            dstColorFactor = srcColor;
            srcAlphaFactor = 0.0f;
            dstAlphaFactor = 1.0f;
            break;
        case moth_ui::BlendMode::Multiply:
            srcColorFactor = dstColor;
            dstColorFactor = { 1.0f - srcColor.a, 1.0f - srcColor.a, 1.0f - srcColor.a, 0.0f };
            srcAlphaFactor = dstColor.a;
            dstAlphaFactor = 1.0f - srcColor.a;
            break;

        }
        float const finalAlpha = (srcColor.a * srcAlphaFactor) + (dstColor.a * dstAlphaFactor);
        moth_ui::Color finalColor = (srcColor * srcColorFactor) + (dstColor * dstColorFactor);
        finalColor.a = finalAlpha;
        return finalColor;
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

