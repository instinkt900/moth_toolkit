#pragma once

#include <string>
#include "moth_ui/utils/serialize_utils.h"
#include <nlohmann/json.hpp>

namespace moth_ui {
    /**
     * @brief Describes a named, bounded animation clip within a Group's timeline.
     *
     * A clip spans a contiguous frame range and specifies the playback rate and
     * loop behaviour.  Multiple clips can share the same track data but define
     * different playable sections.
     */
    struct AnimationClip {
        /**
         * @brief What happens when the clip reaches its last frame.
         */
        enum class LoopType {
            Stop,  ///< Playback stops on the last frame.
            Loop,  ///< Playback wraps back to the first frame.
            Reset, ///< Playback stops and the node resets to frame 0.
        };

        static constexpr float DefaultFPS = 30; ///< Default playback rate in frames per second.

        std::string m_name;                   ///< Unique name identifying this clip.
        int m_startFrame = 0;                 ///< Inclusive start frame index.
        int m_endFrame = 0;                   ///< Inclusive end frame index.
        float m_fps = DefaultFPS;             ///< Playback rate in frames per second.
        LoopType m_loopType = LoopType::Stop; ///< What to do when the clip ends.

        /// @brief Returns the number of frames in this clip (inclusive of both endpoints).
        int FrameCount() const { return m_endFrame - m_startFrame + 1; }

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
