#pragma once

#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    /**
     * @brief Pairs a Node with the AnimationClip currently playing on it.
     *
     * Used internally to track which clip is active on a given node so that
     * EventAnimationStopped can be fired with the correct clip name when
     * playback is interrupted.
     */
    struct ClipController {
        Node* m_node = nullptr;         ///< Node on which the clip is playing.
        AnimationClip* m_clip = nullptr; ///< Clip currently playing, or @c nullptr.
    };
}
