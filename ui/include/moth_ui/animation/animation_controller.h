#pragma once

#include "moth_ui/ui_fwd.h"

#include <vector>
#include <memory>

namespace moth_ui {
    /**
     * @brief Drives all AnimationTrackControllers belonging to a single Node.
     *
     * When SetFrame() is called the controller evaluates each track at the
     * given frame and writes the resulting values to the node's layout rect
     * and colour fields.
     */
    class AnimationController {
    public:
        /**
         * @brief Constructs the controller for a specific node.
         * @param node Node whose layout entity's tracks are to be driven.
         */
        AnimationController(Node* node);
        ~AnimationController();

        /**
         * @brief Evaluates all tracks at the given frame and applies them to the node.
         * @param frame Frame index (may be fractional).
         */
        void SetFrame(float frame);

        AnimationController(AnimationController const&) = delete;
        AnimationController(AnimationController&&) = default;
        AnimationController& operator=(AnimationController const&) = delete;
        AnimationController& operator=(AnimationController&&) = default;

    private:
        Node* m_node = nullptr;
        std::vector<std::unique_ptr<AnimationTrackController>> m_trackControllers;
    };
}
