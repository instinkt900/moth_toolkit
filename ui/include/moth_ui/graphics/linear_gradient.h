#pragma once

#include "moth_ui/utils/color.h"
#include "moth_ui/utils/vector.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

namespace moth_ui {
    /**
     * @brief Parameters describing a two-stop linear colour gradient.
     *
     * Drawn into an arbitrary destination rectangle. The midpoint locates the
     * t=0.5 point inside the rect as a normalised fraction, @c angle gives the
     * gradient direction in radians (0 = +x toward @c endColor), and
     * @c transitionLength scales the length of the lerp along the axis.
     *
     * - @c transitionLength = 1.0 fills the rect.
     * - @c transitionLength = 0.0 produces a sharp step at @c midpoint.
     * - @c transitionLength > 1.0 leaves the lerp partially outside the rect.
     */
    struct LinearGradient {
        Color startColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        Color endColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        FloatVec2 midpoint{ 0.5f, 0.5f };
        float angle = 0.0f;
        float transitionLength = 1.0f;
    };
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
