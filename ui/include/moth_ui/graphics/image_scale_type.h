#pragma once

namespace moth_ui {
    /**
     * @brief Describes how an image is scaled to fill its destination rectangle.
     */
    enum class ImageScaleType {
        Stretch,   ///< Image is stretched uniformly to fill the destination.
        Tile,      ///< Image is tiled to fill the destination.
        NineSlice  ///< Image is nine-sliced so borders are not distorted.
    };
}
