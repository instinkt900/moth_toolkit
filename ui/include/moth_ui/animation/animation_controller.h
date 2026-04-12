#pragma once

#include "moth_ui/animation/animation_track.h"
#include "moth_ui/moth_ui_fwd.h"

#include <functional>
#include <string_view>
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
         *             The node must outlive this controller; the controller
         *             is always owned by the node itself so this invariant
         *             holds by construction.
         */
        AnimationController(Node* node);
        ~AnimationController();

        /**
         * @brief Evaluates all tracks at the given frame and applies them to the node.
         * @param frame Frame index (may be fractional).
         */
        void SetFrame(float frame);

        /**
         * @brief Evaluates only discrete tracks at @p frame, firing change callbacks.
         *
         * Used by Node::Refresh() (editor scrub) which handles continuous tracks itself.
         * @param frame Frame index (may be fractional).
         */
        void SetFrameDiscrete(float frame);

        /// @brief Removes all registered discrete callbacks (used before re-registering on reload).
        void ClearDiscreteCallbacks();

        /**
         * @brief Registers a callback to be invoked when a discrete track changes value.
         *
         * If the entity has no discrete track for @p target the registration is a no-op.
         * @param target   The discrete track target to observe.
         * @param callback Called with the new string value when it changes at a new frame.
         */
        void RegisterDiscreteCallback(AnimationTrack::Target target, std::function<void(std::string_view)> callback);

        AnimationController(AnimationController const&) = delete;
        AnimationController(AnimationController&&) = default;
        AnimationController& operator=(AnimationController const&) = delete;
        AnimationController& operator=(AnimationController&&) = default;

    private:
        Node* m_node = nullptr;
        std::vector<std::unique_ptr<AnimationTrackController>> m_trackControllers;
        std::vector<std::unique_ptr<DiscreteAnimationTrackController>> m_discreteControllers;
    };
}
