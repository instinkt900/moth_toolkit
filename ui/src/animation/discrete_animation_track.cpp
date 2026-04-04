#include "common.h"
#include "moth_ui/animation/discrete_animation_track.h"

#include <nlohmann/json.hpp>
#include <algorithm>

namespace moth_ui {
    DiscreteAnimationTrack::DiscreteAnimationTrack(AnimationTrack::Target target)
        : m_target(target) {
    }

    static std::string const s_emptyString;

    std::string const& DiscreteAnimationTrack::GetValueAtFrame(int frame) const {
        // Walk backwards to find the last keyframe at or before frame.
        for (auto it = m_keyframes.rbegin(); it != m_keyframes.rend(); ++it) {
            if (it->first <= frame) {
                return it->second;
            }
        }
        return s_emptyString;
    }

    std::string& DiscreteAnimationTrack::GetOrCreateKeyframe(int frame) {
        // Find or insert in sorted order.
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), [frame](auto const& kf) {
            return kf.first >= frame;
        });
        if (it != m_keyframes.end() && it->first == frame) {
            return it->second;
        }
        auto inserted = m_keyframes.insert(it, std::make_pair(frame, std::string{}));
        return inserted->second;
    }

    std::string* DiscreteAnimationTrack::GetKeyframe(int frame) {
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), [frame](auto const& kf) {
            return kf.first == frame;
        });
        if (it != m_keyframes.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void DiscreteAnimationTrack::DeleteKeyframe(int frame) {
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), [frame](auto const& kf) {
            return kf.first == frame;
        });
        if (it != m_keyframes.end()) {
            m_keyframes.erase(it);
        }
    }

    void DiscreteAnimationTrack::SortKeyframes() {
        std::sort(m_keyframes.begin(), m_keyframes.end(), [](auto const& a, auto const& b) {
            return a.first < b.first;
        });
    }

    void to_json(nlohmann::json& json, DiscreteAnimationTrack const& track) {
        json["target"] = track.m_target;
        auto keyframesJson = nlohmann::json::array();
        for (auto const& [frame, value] : track.m_keyframes) {
            keyframesJson.push_back({ { "frame", frame }, { "value", value } });
        }
        json["keyframes"] = keyframesJson;
    }

    void from_json(nlohmann::json const& json, DiscreteAnimationTrack& track) {
        track.m_target = json.value("target", AnimationTrack::Target::Unknown);
        track.m_keyframes.clear();
        if (json.contains("keyframes")) {
            for (auto const& kfJson : json["keyframes"]) {
                int frame = kfJson.value("frame", 0);
                std::string value = kfJson.value("value", std::string{});
                track.m_keyframes.emplace_back(frame, std::move(value));
            }
        }
    }
}
