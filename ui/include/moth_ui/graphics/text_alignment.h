#pragma once

namespace moth_ui {
    /// @brief Horizontal alignment of text within its bounding rectangle.
    enum class TextHorizAlignment {
        Left,   ///< Text is left-aligned.
        Center, ///< Text is centred horizontally.
        Right   ///< Text is right-aligned.
    };

    /// @brief Vertical alignment of text within its bounding rectangle.
    enum class TextVertAlignment {
        Top,    ///< Text is aligned to the top edge.
        Middle, ///< Text is centred vertically.
        Bottom  ///< Text is aligned to the bottom edge.
    };
}
