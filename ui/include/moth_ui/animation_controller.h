#pragma once

#include "moth_ui/animation_track.h"
#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    class AnimationTrackController {
    public:
        AnimationTrackController(float& target, AnimationTrack& track);

        auto GetTarget() const { return m_track.GetTarget(); }
        void SetTime(float time);
        void ForEvents(float startTime, float endTime, std::function<void(Keyframe const&)> const& eventCallback);

    private:
        float& m_target;
        AnimationTrack& m_track;
    };

    class AnimationController {
    public:
        AnimationController(Node* node, std::map<AnimationTrack::Target, std::unique_ptr<AnimationTrack>> const& tracks);

        auto GetTime() const { return m_time; }
        auto GetClip() const { return m_clip; }

        void SetClip(AnimationClip* clip);
        void Update(float delta);

    private:
        Node* m_node = nullptr;
        AnimationClip* m_clip = nullptr;
        std::vector<std::unique_ptr<AnimationTrackController>> m_trackControllers;
        float m_time = 0.0f;

        void CheckEvents(float startTime, float endTime);
    };
}
