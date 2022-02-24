#pragma once

namespace ui {
    struct AnimationClip {
        AnimationClip() = default;

        void SetStartTime(float time) {
            m_startTime = time;
            m_endTime = m_startTime + GetDuration();
        }

        float GetDuration() const { return (m_endFrame - m_startFrame) / m_fps; }

        enum class LoopType {
            Stop,
            Loop,
            Reset,
        };

        std::string m_name;
        int m_startFrame = 0;
        int m_endFrame = 0;
        float m_fps = 30;
        LoopType m_loopType = LoopType::Stop;

        float m_startTime = 0;
        float m_endTime = 0;

        bool operator==(AnimationClip const& other) {
            return m_name == other.m_name && m_startFrame == other.m_startFrame && m_endFrame == other.m_endFrame && m_fps == other.m_fps && m_loopType == other.m_loopType;
        }

        bool operator!=(AnimationClip const& other) {
            return !(*this == other);
        }

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(AnimationClip, m_name, m_startFrame, m_endFrame, m_fps, m_loopType);
    };
}
