#pragma once

#include "moth_graphics/utils/vector.h"

#include <string_view>

namespace moth::gfx::graphics {
    /// @brief Abstract handle to a loaded font.
    ///
    /// Holds a reference to a backend-specific font resource. Obtain instances
    /// via @c FontFactory::GetFont() and pass them to @c IGraphics::DrawText().
    class IFont {
    public:
        virtual ~IFont() = default;

        /// @brief Measure the pixel dimensions of a string rendered with this font.
        /// @param text UTF-8 text to measure.
        /// @return Width and height in pixels.
        virtual IntVec2 Measure(std::string_view text) const = 0;

        /// @brief Measure the dimensions of text wrapped to a maximum width.
        ///
        /// The text is word-wrapped to @p maxWidth pixels. The returned width is
        /// the widest line and the height spans every line.
        /// @param text UTF-8 text to measure.
        /// @param maxWidth Maximum line width in pixels (<= 0 = no wrap).
        /// @return Width and height in pixels.
        virtual IntVec2 MeasureWrapped(std::string_view text, int maxWidth) const = 0;

        /// @brief Returns the distance between consecutive baselines in pixels.
        virtual int GetLineHeight() const = 0;

        /// @brief Returns the distance from the baseline to the top of the tallest glyph in pixels.
        virtual int GetAscent() const = 0;

        /// @brief Returns the distance from the baseline to the bottom of the lowest descender in pixels.
        virtual int GetDescent() const = 0;
    };
}
