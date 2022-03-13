#pragma once

#include <variant>
#include "moth_ui/utils/interp.h"
#include "moth_ui/utils/imgui_ext.h"

namespace nlohmann {
    template <>
    struct adl_serializer<std::variant<float, std::string>> {
        static void from_json(json const& j, std::variant<float, std::string>& var) {
            size_t const index = j["index"];
            if (index == 0) {
                var = static_cast<float>(j["value"]);
            } else if (index == 1) {
                var = static_cast<std::string>(j["value"]);
            }
        }

        static void to_json(json& j, std::variant<float, std::string> const& var) {
            j["index"] = var.index();
            std::visit([&](auto&& value) {
                j["value"] = value;
            },
                       var);
        }
    };
}

namespace moth_ui {
    struct AnimationClip;

    using KeyframeValue = std::variant<float, std::string>;

    class Keyframe {
    public:
        int m_frame = 0;
        KeyframeValue m_value;
        InterpType m_interpType = InterpType::Linear;

        float m_time = 0; // calculated based on clips

        float GetFloatValue() const { return std::get<float>(m_value); }
        std::string const& GetStringValue() const { return std::get<std::string>(m_value); }

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Keyframe, m_frame, m_value, m_interpType);
    };

    class AnimationEvent {
    public:
        int m_frame;
        std::string m_name;

        float m_time; // calculated based on clips

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(AnimationEvent, m_frame, m_name);
    };

    class AnimationTrack {
    public:
        enum class Target {
            Unknown,
            Events,
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

        AnimationTrack() = default;
        AnimationTrack(Target target);
        AnimationTrack(Target target, float initialValue);
        explicit AnimationTrack(nlohmann::json const& json);

        Target GetTarget() const { return m_target; }

        Keyframe* GetKeyframe(int frameNo);
        Keyframe& GetOrCreateKeyframe(int frameNo);
        void DeleteKeyframe(int frameNo);
        void DeleteKeyframe(Keyframe* frame);

        void ForKeyframesOverTime(float startTime, float endTime, std::function<void(Keyframe const&)> const& callback);

        void UpdateTrackTimings(std::vector<std::unique_ptr<AnimationClip>> const& clips);
        float GetValueAtTime(float time) const;
        float GetValueAtFrame(int frame) const;

        auto& GetKeyframes() { return m_keyframes; }

        void SortKeyframes();

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(AnimationTrack, m_target, m_keyframes);

    private:
        Target m_target;
        std::vector<Keyframe> m_keyframes;
    };
}
