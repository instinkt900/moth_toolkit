#pragma once

#include "moth_ui/ui_fwd.h"

#include <vector>
#include <memory>

namespace moth_ui {
    class AnimationController {
    public:
        AnimationController(Node* node);
        ~AnimationController();

        void SetFrame(float frame);

        AnimationController(AnimationController const&) = default;
        AnimationController(AnimationController&&) = default;
        AnimationController& operator=(AnimationController const&) = default;
        AnimationController& operator=(AnimationController&&) = default;

    private:
        Node* m_node = nullptr;
        std::vector<std::unique_ptr<AnimationTrackController>> m_trackControllers;
    };
}
