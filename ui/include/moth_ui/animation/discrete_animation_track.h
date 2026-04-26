#pragma once

#include "moth_ui/animation/animation_track.h"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>
#include <utility>

namespace moth_ui {
    /**
     * @brief A non-interpolated animation track that stores string values at specific frames.
     *
     * Unlike AnimationTrack (which interpolates floats), a DiscreteAnimationTrack
     * steps instantly to the value of the last keyframe at or before the current
     * frame. Suitable for targets such as FlipbookClip (clip name) and
     * FlipbookPlaying (play/pause state).
     */
    class DiscreteAnimationTrack {
    public:
        /// @brief Ordered list of (frame, value) pairs sorted ascending by frame.
        using KeyframeList = std::vector<std::pair<int, std::string>>;

        /**
         * @brief Constructs a discrete track for the given animation target.
         * @param target The property this track animates.
         */
        explicit DiscreteAnimationTrack(AnimationTrack::Target target);
        DiscreteAnimationTrack(DiscreteAnimationTrack const& other) = default;
        DiscreteAnimationTrack(DiscreteAnimationTrack&&) = default;
        DiscreteAnimationTrack& operator=(DiscreteAnimationTrack const&) = default;
        DiscreteAnimationTrack& operator=(DiscreteAnimationTrack&&) = default;
        ~DiscreteAnimationTrack() = default;

        /// @brief Returns the property this track animates.
        AnimationTrack::Target GetTarget() const { return m_target; }

        /// @brief Returns a mutable reference to the keyframe list.
        KeyframeList& Keyframes() { return m_keyframes; }

        /// @brief Returns a const reference to the keyframe list.
        KeyframeList const& Keyframes() const { return m_keyframes; }

        /**
         * @brief Returns the value at the last keyframe at or before @p frame.
         *
         * Returns an empty string if no keyframe precedes the given frame.
         * @param frame Frame index.
         * @return Const reference to the value at or before @p frame.
         */
        std::string const& GetValueAtFrame(int frame) const;

        /**
         * @brief Returns a reference to the value at @p frame, creating an empty entry if absent.
         * @param frame Frame index.
         * @return Reference to the existing or newly created value.
         */
        std::string& GetOrCreateKeyframe(int frame);

        /**
         * @brief Returns a pointer to the value at the given frame, or @c nullptr if absent.
         * @param frame Frame index.
         * @return Pointer to the value, or @c nullptr if none exists at @p frame.
         */
        std::string* GetKeyframe(int frame);

        /**
         * @brief Deletes the keyframe at the given frame number.
         * @param frame Frame index.
         */
        void DeleteKeyframe(int frame);

        /// @brief Sorts keyframes by ascending frame index.
        void SortKeyframes();

        friend void to_json(nlohmann::json& json, DiscreteAnimationTrack const& track);
        friend void from_json(nlohmann::json const& json, DiscreteAnimationTrack& track);

    private:
        AnimationTrack::Target m_target = AnimationTrack::Target::Unknown;
        KeyframeList m_keyframes;
    };
}
