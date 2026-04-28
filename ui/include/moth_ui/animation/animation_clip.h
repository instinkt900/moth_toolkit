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

        std::string name;                   ///< Unique name identifying this clip.
        int startFrame = 0;                 ///< Inclusive start frame index.
        int endFrame = 0;                   ///< Inclusive end frame index.
        float fps = DefaultFPS;             ///< Playback rate in frames per second.
        LoopType loopType = LoopType::Stop; ///< What to do when the clip ends.

        /// @brief Returns the number of frames in this clip (inclusive of both endpoints).
        int FrameCount() const { return endFrame - startFrame + 1; }

        bool operator==(AnimationClip const& other) const {
            return name == other.name && startFrame == other.startFrame && endFrame == other.endFrame && fps == other.fps && loopType == other.loopType;
        }

        bool operator!=(AnimationClip const& other) const {
            return !(*this == other);
        }

        /// @brief Serializes this clip to JSON.
        friend void to_json(nlohmann::json& j, AnimationClip const& clip) {
            j["name"] = clip.name;
            j["startFrame"] = clip.startFrame;
            j["endFrame"] = clip.endFrame;
            j["fps"] = clip.fps;
            j["loopType"] = clip.loopType;
        }

        /// @brief Deserializes a clip from JSON.
        friend void from_json(nlohmann::json const& j, AnimationClip& clip) {
            // TODO: Remove m_-prefixed fallback keys in a future version.
            clip.name = j.value("name", j.value("m_name", ""));
            clip.startFrame = j.value("startFrame", j.value("m_startFrame", 0));
            clip.endFrame = j.value("endFrame", j.value("m_endFrame", 0));
            clip.fps = j.value("fps", j.value("m_fps", DefaultFPS));
            clip.loopType = j.value("loopType", j.value("m_loopType", LoopType::Stop));
        }

        AnimationClip() = default;
        AnimationClip(AnimationClip const&) = default;
        AnimationClip(AnimationClip&&) = default;
        AnimationClip& operator=(AnimationClip const&) = default;
        AnimationClip& operator=(AnimationClip&&) = default;
        ~AnimationClip() = default;
    };
}
