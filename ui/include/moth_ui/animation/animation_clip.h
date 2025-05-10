#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace moth_ui {
    struct AnimationClip {
        enum class LoopType {
            Stop,
            Loop,
            Reset,
        };

        static constexpr float DefaultFPS = 30;
        std::string m_name;
        int m_startFrame = 0;
        int m_endFrame = 0;
        float m_fps = DefaultFPS;
        LoopType m_loopType = LoopType::Stop;

        int FrameCount() const { return m_endFrame - m_startFrame; }

        bool operator==(AnimationClip const& other) const {
            return m_name == other.m_name && m_startFrame == other.m_startFrame && m_endFrame == other.m_endFrame && m_fps == other.m_fps && m_loopType == other.m_loopType;
        }

        bool operator!=(AnimationClip const& other) const {
            return !(*this == other);
        }

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(AnimationClip, m_name, m_startFrame, m_endFrame, m_fps, m_loopType);

        AnimationClip() = default;
        AnimationClip(AnimationClip const&) = default;
        AnimationClip(AnimationClip&&) = default;
        AnimationClip& operator=(AnimationClip const&) = default;
        AnimationClip& operator=(AnimationClip&&) = default;
        ~AnimationClip() = default;
    };
}
