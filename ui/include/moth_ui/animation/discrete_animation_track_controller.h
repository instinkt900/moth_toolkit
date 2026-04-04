#pragma once

#include "moth_ui/animation/discrete_animation_track.h"

#include <functional>
#include <string_view>

namespace moth_ui {
    /**
     * @brief Drives a DiscreteAnimationTrack, invoking a callback only when the value changes.
     *
     * The callback receives the new value as a @c std::string_view whenever
     * SetFrame() moves into a region with a different step value.
     */
    class DiscreteAnimationTrackController {
    public:
        /**
         * @brief Constructs the controller.
         * @param track    The discrete track to evaluate.
         * @param callback Called with the new value when it changes at a new frame.
         */
        DiscreteAnimationTrackController(DiscreteAnimationTrack const& track, std::function<void(std::string_view)> callback);

        DiscreteAnimationTrackController(DiscreteAnimationTrackController const&) = delete;
        DiscreteAnimationTrackController(DiscreteAnimationTrackController&&) = default;
        DiscreteAnimationTrackController& operator=(DiscreteAnimationTrackController const&) = delete;
        DiscreteAnimationTrackController& operator=(DiscreteAnimationTrackController&&) = default;
        ~DiscreteAnimationTrackController() = default;

        /**
         * @brief Evaluates the track at @p frame and invokes the callback if the value changed.
         * @param frame Frame index (truncated to int for step evaluation).
         */
        void SetFrame(float frame);

        /// @brief Resets the last-seen value so the callback fires on the next SetFrame().
        void Reset();

    private:
        DiscreteAnimationTrack const* m_track = nullptr;
        std::function<void(std::string_view)> m_callback;
        std::string m_lastValue;
        bool m_initialized = false;
    };
}
