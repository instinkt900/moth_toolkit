#pragma once

#include "moth_ui/moth_ui_fwd.h"

#include <nlohmann/json.hpp>
#include <array>
#include <functional>
#include <memory>
#include <vector>

namespace moth_ui {
    /**
     * @brief A single animated property track consisting of an ordered list of keyframes.
     *
     * Each track targets one property of a layout entity (e.g. left offset or
     * colour red component) and evaluates the interpolated value at any frame.
     */
    class AnimationTrack {
    public:
        /// @brief Ordered list of keyframes (shared ownership for editor use).
        using KeyframeList = std::vector<std::shared_ptr<Keyframe>>;

        /// @brief The layout property driven by this track.
        enum class Target {
            Unknown,       ///< Unrecognised or unset target.
            Events,        ///< Event keyframe track (not a float value target; ignored by AnimationController).
            TopOffset,     ///< Top edge pixel offset.
            BottomOffset,  ///< Bottom edge pixel offset.
            LeftOffset,    ///< Left edge pixel offset.
            RightOffset,   ///< Right edge pixel offset.
            TopAnchor,     ///< Top anchor fraction [0,1].
            BottomAnchor,  ///< Bottom anchor fraction [0,1].
            LeftAnchor,    ///< Left anchor fraction [0,1].
            RightAnchor,   ///< Right anchor fraction [0,1].
            ColorRed,      ///< Red colour component [0,1].
            ColorGreen,    ///< Green colour component [0,1].
            ColorBlue,     ///< Blue colour component [0,1].
            ColorAlpha,    ///< Alpha colour component [0,1].
            Rotation,      ///< Clockwise rotation in degrees.
            FlipbookClip,    ///< Flipbook clip name (discrete string, step-interpolated).
            FlipbookPlaying, ///< Flipbook play/pause state as "1"/"0" (discrete, step-interpolated).
        };

        /// @brief All targets that are continuously interpolated (excludes Unknown).
        static constexpr std::array<Target, 13> ContinuousTargets{
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
            Target::Rotation,
        };

        /// @brief All targets that are step-interpolated string values (discrete tracks).
        static constexpr std::array<Target, 2> DiscreteTargets{
            Target::FlipbookClip,
            Target::FlipbookPlaying,
        };

        AnimationTrack(AnimationTrack const& other);

        /**
         * @brief Constructs a track for the given target with no keyframes.
         * @param target Property this track will animate.
         */
        explicit AnimationTrack(Target target);

        /**
         * @brief Constructs a track for the given target with a single keyframe at frame 0.
         * @param target       Property this track will animate.
         * @param initialValue Value to store in the initial keyframe.
         */
        AnimationTrack(Target target, float initialValue);

        /**
         * @brief Deserialises a track from JSON.
         * @param json JSON object representing the track.
         */
        explicit AnimationTrack(nlohmann::json const& json);
        AnimationTrack& operator=(AnimationTrack const& other);

        /// @brief Returns the property this track animates.
        Target GetTarget() const { return m_target; }

        /// @brief Returns a mutable reference to the keyframe list.
        KeyframeList& Keyframes() { return m_keyframes; }

        /**
         * @brief Returns the keyframe at the given frame number, or @c nullptr.
         * @param frameNo Exact frame index to look up.
         */
        Keyframe* GetKeyframe(int frameNo);

        /**
         * @brief Returns the keyframe at @p frameNo, creating one if it does not exist.
         * @param frameNo Frame index.
         */
        Keyframe& GetOrCreateKeyframe(int frameNo);

        /**
         * @brief Deletes the keyframe at the given frame number.
         * @param frameNo Frame index of the keyframe to delete.
         */
        void DeleteKeyframe(int frameNo);

        /**
         * @brief Deletes the given keyframe by pointer.
         * @param frame Pointer to the keyframe to remove.
         */
        void DeleteKeyframe(Keyframe* frame);

        /**
         * @brief Invokes @p callback for every keyframe within the given frame range.
         * @param startFrame Inclusive start of the range.
         * @param endFrame   Inclusive end of the range.
         * @param callback   Callable receiving each matching Keyframe.
         */
        void ForKeyframesOverFrames(float startFrame, float endFrame, std::function<void(Keyframe const&)> const& callback);

        /**
         * @brief Evaluates the interpolated track value at the given frame.
         * @param frame Frame index (may be fractional).
         * @return Interpolated scalar value.
         */
        float GetValueAtFrame(float frame) const;

        /// @brief Sorts keyframes by ascending frame index.
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
