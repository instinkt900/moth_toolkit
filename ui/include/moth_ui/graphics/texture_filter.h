#pragma once

namespace moth_ui {
    /**
     * @brief Selects the texture sampling filter applied when an image is
     *        scaled up or down during rendering.
     */
    enum class TextureFilter {
        Invalid = -1, ///< Sentinel value indicating an unset filter.
        Linear,       ///< Bilinear interpolation — smooth, suitable for photos and gradients.
        Nearest,      ///< Nearest-neighbour — crisp, suitable for pixel art and icons.
    };
}
