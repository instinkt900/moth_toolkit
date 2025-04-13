#pragma once

#include "moth_ui/utils/interp.h"

#include <nlohmann/json_fwd.hpp>

#include <variant>
#include <string>
#include <array>
#include <functional>

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

    using KeyframeValue = float;

    class Keyframe {
    public:
        Keyframe() = default;
        Keyframe(int frame, KeyframeValue value)
            : m_frame(frame)
            , m_value(value) {}

        int m_frame = 0;
        KeyframeValue m_value = 0;
        InterpType m_interpType = InterpType::Linear;

        friend void to_json(nlohmann::json& j, Keyframe const& keyframe) {
            j["frame"] = keyframe.m_frame;
            j["value"] = keyframe.m_value;
            j["interp"] = keyframe.m_interpType;
        }

        friend void from_json(nlohmann::json const& j, Keyframe& keyframe) {
            nlohmann::json valueJson = j.at("value");
            if (valueJson.is_number_float()) {
                valueJson.get_to(keyframe.m_value);
            } else {
                std::variant<float, std::string> oldValueType;
                valueJson.get_to(oldValueType);
                keyframe.m_value = std::get<float>(oldValueType);
            }

            j.at("frame").get_to(keyframe.m_frame);
            j.at("interp").get_to(keyframe.m_interpType);
        }
    };

    class AnimationEvent {
    public:
        AnimationEvent() = default;
        AnimationEvent(int frame, std::string const& name)
            : m_frame(frame)
            , m_name(name) {}

        int m_frame = 0;
        std::string m_name;

        bool operator==(AnimationEvent const& other) const {
            return m_frame == other.m_frame && m_name == other.m_name;
        }

        bool operator!=(AnimationEvent const& other) const {
            return !(*this == other);
        }

        friend void to_json(nlohmann::json& j, AnimationEvent const& event) {
            j["frame"] = event.m_frame;
            j["name"] = event.m_name;
        }

        friend void from_json(nlohmann::json const& j, AnimationEvent& event) {
            j.at("frame").get_to(event.m_frame);
            j.at("name").get_to(event.m_name);
        }
    };

    class AnimationTrack {
    public:
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

        AnimationTrack() = default;
        AnimationTrack(AnimationTrack const& other);
        explicit AnimationTrack(Target target);
        AnimationTrack(Target target, float initialValue);
        explicit AnimationTrack(nlohmann::json const& json);

        Target GetTarget() const { return m_target; }

        Keyframe* GetKeyframe(int frameNo);
        Keyframe& GetOrCreateKeyframe(int frameNo);
        void DeleteKeyframe(int frameNo);
        void DeleteKeyframe(Keyframe* frame);

        void ForKeyframesOverFrames(float startFrame, float endFrame, std::function<void(Keyframe const&)> const& callback);

        float GetValueAtFrame(float frame) const;

        void SortKeyframes();

        friend void to_json(nlohmann::json& j, AnimationTrack const& track) {
            j["target"] = track.m_target;
            std::vector<Keyframe> keyframes;
            for (auto& keyframe : track.m_keyframes) {
                keyframes.push_back(*keyframe);
            }
            j["keyframes"] = keyframes;
        }

        friend void from_json(nlohmann::json const& j, AnimationTrack& track) {
            track.m_target = j.value("target", Target::Unknown);
            auto keyframes = j.value("keyframes", std::vector<Keyframe>{});
            for (auto& keyframe : keyframes) {
                track.m_keyframes.push_back(std::make_unique<Keyframe>(keyframe));
            }
        }

        std::vector<std::shared_ptr<Keyframe>> m_keyframes;

    private:
        Target m_target = Target::Unknown;
    };
}
