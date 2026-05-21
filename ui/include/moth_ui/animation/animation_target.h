#pragma once

#include <array>

namespace moth_ui {

/// @brief Properties that can be animated via AnimationTrack or DiscreteAnimationTrack.
enum class AnimationTarget {
    Unknown,             ///< Unrecognised or unset target.
    TopOffset,           ///< Top edge pixel offset.
    BottomOffset,        ///< Bottom edge pixel offset.
    LeftOffset,          ///< Left edge pixel offset.
    RightOffset,         ///< Right edge pixel offset.
    TopAnchor,           ///< Top anchor fraction [0,1].
    BottomAnchor,        ///< Bottom anchor fraction [0,1].
    LeftAnchor,          ///< Left anchor fraction [0,1].
    RightAnchor,         ///< Right anchor fraction [0,1].
    ColorRed,            ///< Red colour component [0,1].
    ColorGreen,          ///< Green colour component [0,1].
    ColorBlue,           ///< Blue colour component [0,1].
    ColorAlpha,          ///< Alpha colour component [0,1].
    Rotation,            ///< Clockwise rotation in degrees.
    FlipbookClip,        ///< Flipbook clip name (discrete string, step-interpolated).
    FlipbookPlaying,     ///< Flipbook play/pause state as "1"/"0" (discrete, step-interpolated).
    GradientStartRed,    ///< Gradient start-colour red [0,1] (NodeGradient only).
    GradientStartGreen,  ///< Gradient start-colour green [0,1] (NodeGradient only).
    GradientStartBlue,   ///< Gradient start-colour blue [0,1] (NodeGradient only).
    GradientStartAlpha,  ///< Gradient start-colour alpha [0,1] (NodeGradient only).
    GradientEndRed,      ///< Gradient end-colour red [0,1] (NodeGradient only).
    GradientEndGreen,    ///< Gradient end-colour green [0,1] (NodeGradient only).
    GradientEndBlue,     ///< Gradient end-colour blue [0,1] (NodeGradient only).
    GradientEndAlpha,    ///< Gradient end-colour alpha [0,1] (NodeGradient only).
    GradientMidpointX,   ///< Gradient midpoint x as a fraction [0,1] of the rect (NodeGradient only).
    GradientMidpointY,   ///< Gradient midpoint y as a fraction [0,1] of the rect (NodeGradient only).
    GradientAngle,       ///< Gradient direction in radians; 0 = +x (NodeGradient only).
    GradientTransition,  ///< Gradient transition length as factor of projected extent (NodeGradient only).
};

/// All AnimationTarget values that are continuously interpolated (excludes Unknown).
inline constexpr std::array<AnimationTarget, 25> kContinuousTargets{
    AnimationTarget::TopOffset,
    AnimationTarget::BottomOffset,
    AnimationTarget::LeftOffset,
    AnimationTarget::RightOffset,
    AnimationTarget::TopAnchor,
    AnimationTarget::BottomAnchor,
    AnimationTarget::LeftAnchor,
    AnimationTarget::RightAnchor,
    AnimationTarget::ColorRed,
    AnimationTarget::ColorGreen,
    AnimationTarget::ColorBlue,
    AnimationTarget::ColorAlpha,
    AnimationTarget::Rotation,
    AnimationTarget::GradientStartRed,
    AnimationTarget::GradientStartGreen,
    AnimationTarget::GradientStartBlue,
    AnimationTarget::GradientStartAlpha,
    AnimationTarget::GradientEndRed,
    AnimationTarget::GradientEndGreen,
    AnimationTarget::GradientEndBlue,
    AnimationTarget::GradientEndAlpha,
    AnimationTarget::GradientMidpointX,
    AnimationTarget::GradientMidpointY,
    AnimationTarget::GradientAngle,
    AnimationTarget::GradientTransition,
};

/// All AnimationTarget values that are step-interpolated string values (discrete tracks).
inline constexpr std::array<AnimationTarget, 2> kDiscreteTargets{
    AnimationTarget::FlipbookClip,
    AnimationTarget::FlipbookPlaying,
};

} // namespace moth_ui
