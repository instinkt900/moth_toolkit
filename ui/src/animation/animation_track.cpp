#include "common.h"
#include "moth_ui/animation/animation_track.h"
#include "moth_ui/animation/keyframe.h"

namespace moth_ui {
    void to_json(nlohmann::json& json, AnimationTrack const& track) {
        json["target"] = track.m_target;
        std::vector<Keyframe> keyframes;
        for (auto const& keyframe : track.m_keyframes) {
            keyframes.push_back(*keyframe);
        }
        json["keyframes"] = keyframes;
    }

    void from_json(nlohmann::json const& json, AnimationTrack& track) {
        if (json.value("target", "") == "Events") {
            // Target::Events was removed. Silently discard legacy event tracks.
            track.m_target = AnimationTrack::Target::Unknown;
        } else {
            track.m_target = json.value("target", AnimationTrack::Target::Unknown);
        }
        auto keyframes = json.value("keyframes", std::vector<Keyframe>{});
        track.m_keyframes.clear();
        for (auto& keyframe : keyframes) {
            track.m_keyframes.push_back(std::make_unique<Keyframe>(keyframe));
        }
    }

    AnimationTrack::AnimationTrack(AnimationTrack const& other)
        : m_target(other.m_target) {
        for (auto&& keyframe : other.m_keyframes) {
            m_keyframes.push_back(std::make_shared<Keyframe>(*keyframe));
        }
    }

    AnimationTrack& AnimationTrack::operator=(AnimationTrack const& other) {
        if (this != &other) {
            m_target = other.m_target;
            m_keyframes.clear();
            for (auto&& keyframe : other.m_keyframes) {
                m_keyframes.push_back(std::make_shared<Keyframe>(*keyframe));
            }
        }
        return *this;
    }

    AnimationTrack::AnimationTrack(Target target)
        : m_target(target) {
    }

    AnimationTrack::AnimationTrack(Target target, float initialValue)
        : m_target(target) {
        m_keyframes.push_back(std::make_unique<Keyframe>(0, initialValue));
    }

    AnimationTrack::AnimationTrack(nlohmann::json const& json) {
        *this = json;
        SortKeyframes();
    }

    Keyframe* AnimationTrack::GetKeyframe(int frameNo) {
        auto keyframeIt = ranges::find_if(m_keyframes, [&](auto const& keyf) { return keyf->frame == frameNo; });
        if (std::end(m_keyframes) != keyframeIt) {
            return keyframeIt->get();
        }
        return nullptr;
    }

    Keyframe const* AnimationTrack::GetKeyframe(int frameNo) const {
        auto keyframeIt = ranges::find_if(m_keyframes, [&](auto const& keyf) { return keyf->frame == frameNo; });
        if (std::end(m_keyframes) != keyframeIt) {
            return keyframeIt->get();
        }
        return nullptr;
    }

    Keyframe& AnimationTrack::GetOrCreateKeyframe(int frameNo) {
        // find the frame or the first iterator after where it would be
        auto keyframeIt = ranges::find_if(m_keyframes, [&](auto const& keyf) { return keyf->frame >= frameNo; });
        if (std::end(m_keyframes) != keyframeIt && (*keyframeIt)->frame == frameNo) {
            // found an existing frame
            return *(*keyframeIt);
        }

        // didnt find the frame. keyframeIt will be one above the new one
        auto newFrameIt = m_keyframes.insert(keyframeIt, std::make_unique<Keyframe>(frameNo, KeyframeValue{}));
        return *(*newFrameIt);
    }

    void AnimationTrack::DeleteKeyframe(int frameNo) {
        auto keyframeIt = ranges::find_if(m_keyframes, [&](auto const& keyf) { return keyf->frame == frameNo; });
        if (std::end(m_keyframes) != keyframeIt) {
            m_keyframes.erase(keyframeIt);
        }
    }

    void AnimationTrack::DeleteKeyframe(Keyframe* frame) {
        auto keyframeIt = ranges::find_if(m_keyframes, [&](auto const& keyf) { return keyf.get() == frame; });
        if (std::end(m_keyframes) != keyframeIt) {
            m_keyframes.erase(keyframeIt);
        }
    }

    void AnimationTrack::ForKeyframesOverFrames(float startFrame, float endFrame, std::function<void(Keyframe const&)> const& callback) {
        for (auto& keyframe : m_keyframes) {
            int const startF = static_cast<int>(startFrame);
            int const endF = static_cast<int>(endFrame);
            if (keyframe->frame > startF && keyframe->frame <= endF) {
                callback(*keyframe);
            }
        }
    }

    float AnimationTrack::GetValueAtFrame(float frame) const {
        if (m_keyframes.empty()) {
            return 0.0f;
        }

        auto second = std::lower_bound(std::begin(m_keyframes), std::end(m_keyframes), frame,
            [](auto const& kf, float f) { return static_cast<float>(kf->frame) < f; });

        if (second == std::begin(m_keyframes)) {
            return (*second)->value;
        }

        if (second == std::end(m_keyframes)) {
            return m_keyframes.back()->value;
        }

        auto first = std::prev(second);
        float const deltaFrames = frame - static_cast<float>((*first)->frame);
        float const totalFrames = static_cast<float>((*second)->frame - (*first)->frame);
        float const factor = totalFrames == 0.0f ? 0.0f : deltaFrames / totalFrames;
        return Interp((*first)->value, (*second)->value, factor, (*first)->interpType);
    }

    void AnimationTrack::SortKeyframes() {
        ranges::sort(m_keyframes, [](auto const& kfa, auto const& kfb) {
            return kfa->frame < kfb->frame;
        });
    }
}
