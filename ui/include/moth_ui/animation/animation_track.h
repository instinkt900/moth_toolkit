#pragma once

#include "moth_ui/ui_fwd.h"

#include <nlohmann/json.hpp>
#include <array>
#include <functional>
#include <memory>
#include <vector>

namespace moth_ui {
    class AnimationTrack {
    public:
        using KeyframeList = std::vector<std::shared_ptr<Keyframe>>;

        enum class Target {
            Unknown,
            TopOffset,
            BottomOffset,
            LeftOffset,
            RightOffset,
            TopAnchor,
            BottomAnchor,
            LeftAnchor,
            RightAnchor,
            ColorRed,
            ColorGreen,
            ColorBlue,
            ColorAlpha,
        };

        static constexpr std::array<Target, 12> ContinuousTargets{
            Target::TopOffset,
            Target::BottomOffset,
            Target::LeftOffset,
            Target::RightOffset,
            Target::TopAnchor,
            Target::BottomAnchor,
            Target::LeftAnchor,
            Target::RightAnchor,
            Target::ColorRed,
            Target::ColorGreen,
            Target::ColorBlue,
            Target::ColorAlpha,
        };

        AnimationTrack(AnimationTrack const& other);
        explicit AnimationTrack(Target target);
        AnimationTrack(Target target, float initialValue);
        explicit AnimationTrack(nlohmann::json const& json);
        AnimationTrack& operator=(AnimationTrack const&);

        Target GetTarget() const { return m_target; }

        KeyframeList& Keyframes() { return m_keyframes; }
        Keyframe* GetKeyframe(int frameNo);
        Keyframe& GetOrCreateKeyframe(int frameNo);
        void DeleteKeyframe(int frameNo);
        void DeleteKeyframe(Keyframe* frame);

        void ForKeyframesOverFrames(float startFrame, float endFrame, std::function<void(Keyframe const&)> const& callback);

        float GetValueAtFrame(float frame) const;

        void SortKeyframes();

        friend void to_json(nlohmann::json& json, AnimationTrack const& track);
        friend void from_json(nlohmann::json const& json, AnimationTrack& track);

        AnimationTrack() = default;
        AnimationTrack(AnimationTrack&& other) = default;
        AnimationTrack& operator=(AnimationTrack&&) = default;
        ~AnimationTrack() = default;

    private:
        Target m_target = Target::Unknown;
        KeyframeList m_keyframes;
    };
}
