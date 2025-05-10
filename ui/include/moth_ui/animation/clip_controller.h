#pragma once

#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    struct ClipController {
        Node* m_node = nullptr;
        AnimationClip* m_clip = nullptr;
    };
}
