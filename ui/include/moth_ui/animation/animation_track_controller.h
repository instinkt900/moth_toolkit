#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/animation/animation_track.h"

namespace moth_ui {
    class AnimationTrackController {
    public:
        AnimationTrackController(float& target, AnimationTrack& track);

        AnimationTrack::Target GetTarget() const;
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
