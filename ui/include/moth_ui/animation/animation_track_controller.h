#pragma once

#include "moth_ui/moth_ui_fwd.h"
#include "moth_ui/animation/animation_track.h"

namespace moth_ui {
    /**
     * @brief Evaluates a single AnimationTrack and writes the result to a float target.
     *
     * AnimationController holds one AnimationTrackController per track.
     * The controller is non-copyable and non-movable because it holds a reference
     * to the output float.
     *
     * @note The references bound at construction are assumed to remain valid for
     * the controller's lifetime. A Node's layout entity is set once at
     * construction and never replaced — changing it after the fact is considered
     * a programming error. Given that invariant the AnimationTrack and the node
     * property float it drives are both stable for the node's lifetime.
     */
    class AnimationTrackController {
    public:
        /**
         * @brief Constructs a controller bound to a specific output and track.
         * @param target Reference to the float that receives interpolated values.
         * @param track  Track to evaluate.
         */
        AnimationTrackController(float& target, AnimationTrack& track);

        /// @brief Returns the target property driven by this controller.
        AnimationTrack::Target GetTarget() const;

        /**
         * @brief Evaluates the track at @p frame and writes the result to the bound target.
         * @param frame Frame index (may be fractional).
         */
        void SetFrame(float frame);

        AnimationTrackController(AnimationTrackController const&) = delete;
        AnimationTrackController(AnimationTrackController&&) = delete;
        AnimationTrackController& operator=(AnimationTrackController const&) = delete;
        AnimationTrackController& operator=(AnimationTrackController&&) = delete;
        ~AnimationTrackController() = default;

    private:
        float& m_target;
        AnimationTrack& m_track;
    };
}
