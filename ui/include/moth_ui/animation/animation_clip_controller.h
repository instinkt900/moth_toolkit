#pragma once

#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    class AnimationClipController {
    public:
        AnimationClipController(Group* group);

        void SetClip(AnimationClip* clip);
        void Update(float deltaSeconds);

        AnimationClipController(AnimationClipController const&) = default;
        AnimationClipController(AnimationClipController&&) = default;
        AnimationClipController& operator=(AnimationClipController const&) = default;
        AnimationClipController& operator=(AnimationClipController&&) = default;
        ~AnimationClipController() = default;

    private:
        Group* m_group;
        AnimationClip* m_clip = nullptr;
        float m_frame = 0.0f;

        void CheckEvents(float startFrame, float endFrame);
    };
}
