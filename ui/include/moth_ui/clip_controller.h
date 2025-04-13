#pragma once

#include "moth_ui/node.h"
#include "moth_ui/animation_clip.h"

namespace moth_ui {
    class ClipController {
    public:
        Node* m_node = nullptr;
        AnimationClip* m_clip = nullptr;
    };
}
