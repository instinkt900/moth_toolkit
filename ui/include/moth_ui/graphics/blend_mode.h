#pragma once

namespace moth_ui {
    /**
     * @brief Controls how a source image is composited over a destination.
     */
    enum class BlendMode {
        Invalid = -1,  ///< Sentinel value indicating an unset blend mode.
        Replace,       ///< Source replaces destination entirely.
        Alpha,         ///< Standard alpha blending (src·alpha + dst·(1−alpha)).
        Add,           ///< Source is added to destination.
        Multiply,      ///< Source and destination are multiplied together.
        Modulate       ///< Destination is modulated by the source colour.
    };
}
