#pragma once

#include "moth_ui/animation_track.h"
#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    class AnimationTrackController {
    public:
        AnimationTrackController(float& target, AnimationTrack& track);

        auto GetTarget() const { return m_track.GetTarget(); }
        void SetFrame(float frame);

    private:
        float& m_target;
        AnimationTrack& m_track;
    };

    class AnimationController {
    public:
        AnimationController(Node* node);

        void SetFrame(float frame);

    private:
        Node* m_node = nullptr;
        std::vector<std::unique_ptr<AnimationTrackController>> m_trackControllers;
    };

    class AnimationClipController {
    public:
        AnimationClipController(Group* group);

        void SetClip(AnimationClip* clip);
        void Update(float deltaSeconds);

    private:
        Group* m_group;
        AnimationClip* m_clip = nullptr;
        float m_frame = 0.0f;

        void CheckEvents(float startFrame, float endFrame);
    };
}
