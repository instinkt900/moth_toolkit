#pragma once

#include "moth_ui/ui_fwd.h"
#include <memory>

namespace moth_ui {
    /**
     * @brief Advances playback of an AnimationClip within a Group.
     *
     * On each Update() call the controller increments the current frame by the
     * clip's fps × deltaSeconds, fires any animation events that were crossed,
     * applies the frame to all child nodes via the Group, and handles looping.
     */
    class AnimationClipController {
    public:
        /**
         * @brief Constructs a controller bound to the given group.
         * @param group Group whose children will be refreshed during playback.
         */
        AnimationClipController(Group* group);

        /**
         * @brief Sets the active animation clip to play.
         * @param clip Clip to play, or @c nullptr to stop playback.
         */
        void SetClip(std::shared_ptr<AnimationClip> clip);

        /**
         * @brief Advances the clip by the given time and applies the new frame.
         * @param deltaSeconds Time elapsed since the last update, in seconds.
         */
        void Update(float deltaSeconds);

        AnimationClipController(AnimationClipController const&) = default;
        AnimationClipController(AnimationClipController&&) = default;
        AnimationClipController& operator=(AnimationClipController const&) = default;
        AnimationClipController& operator=(AnimationClipController&&) = default;
        ~AnimationClipController() = default;

    private:
        Group* m_group;
        std::weak_ptr<AnimationClip> m_clip;
        float m_frame = 0.0f;

        void CheckEvents(float startFrame, float endFrame);
    };
}
