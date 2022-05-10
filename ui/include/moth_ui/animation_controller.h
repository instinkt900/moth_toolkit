#pragma once

#include "moth_ui/animation_track.h"
#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    class AnimationTrackController {
    public:
        AnimationTrackController(float& target, AnimationTrack& track);

        auto GetTarget() const { return m_track.GetTarget(); }
        void SetFrame(float frame);
        void ForEvents(float startFrame, float endFrame, std::function<void(Keyframe const&)> const& eventCallback);

    private:
        float& m_target;
        AnimationTrack& m_track;
    };

    class AnimationController {
    public:
        AnimationController(Node* node, std::map<AnimationTrack::Target, std::unique_ptr<AnimationTrack>> const& tracks);

        auto GetFrame() const { return m_frame; }
        auto GetClip() const { return m_clip; }

        void SetClip(AnimationClip* clip, bool notifyParentOnFinish);
        void Update(float deltaSeconds);

    private:
        Node* m_node = nullptr;
        AnimationClip* m_clip = nullptr;
        std::vector<std::unique_ptr<AnimationTrackController>> m_trackControllers;
        float m_frame = 0.0f;
        bool m_notify = false;

        void CheckEvents(float startTime, float endTime);
    };
}
