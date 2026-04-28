// Pins the animation system types: Keyframe, AnimationTrack, AnimationClip, AnimationMarker.

#include "moth_ui/moth_ui.h"

#include <catch2/catch_all.hpp>
#include <type_traits>

using namespace moth_ui;

namespace {
    // std::array::operator== is only constexpr from C++20; provide a C++17 helper.
    template <typename T, std::size_t N>
    constexpr bool arrays_equal(std::array<T, N> const& a, std::array<T, N> const& b) {
        for (std::size_t i = 0; i < N; ++i) {
            if (a[i] != b[i]) { return false; }
        }
        return true;
    }

    // ContinuousTargets
    using Target = AnimationTrack::Target;
    constexpr std::array expectedContinuous = {
        Target::TopOffset,    Target::BottomOffset, Target::LeftOffset,  Target::RightOffset,
        Target::TopAnchor,    Target::BottomAnchor, Target::LeftAnchor,  Target::RightAnchor,
        Target::ColorRed,     Target::ColorGreen,   Target::ColorBlue,   Target::ColorAlpha,
        Target::Rotation
    };
    static_assert(arrays_equal(AnimationTrack::ContinuousTargets, expectedContinuous),
                  "AnimationTrack::ContinuousTargets contents or order changed");

    // DiscreteTargets
    constexpr std::array expectedDiscrete = {
        Target::FlipbookClip,
        Target::FlipbookPlaying,
    };
    static_assert(arrays_equal(AnimationTrack::DiscreteTargets, expectedDiscrete),
                  "AnimationTrack::DiscreteTargets contents or order changed");

    // Keyframe fields
    static_assert(std::is_same_v<decltype(Keyframe::frame),     int>);
    static_assert(std::is_same_v<decltype(Keyframe::value),     float>);
    static_assert(std::is_same_v<decltype(Keyframe::interpType), InterpType>);

    // AnimationClip fields
    static_assert(std::is_same_v<decltype(AnimationClip::name),      std::string>);
    static_assert(std::is_same_v<decltype(AnimationClip::startFrame), int>);
    static_assert(std::is_same_v<decltype(AnimationClip::endFrame),   int>);
    static_assert(std::is_same_v<decltype(AnimationClip::fps),        float>);
    static_assert(std::is_same_v<decltype(AnimationClip::loopType),   AnimationClip::LoopType>);

    // AnimationMarker fields
    static_assert(std::is_same_v<decltype(AnimationMarker::frame), int>);
    static_assert(std::is_same_v<decltype(AnimationMarker::name),  std::string>);

    // Target enum distinctness
    static_assert(Target::TopOffset    != Target::BottomOffset);
    static_assert(Target::LeftOffset   != Target::RightOffset);
    static_assert(Target::TopAnchor    != Target::BottomAnchor);
    static_assert(Target::LeftAnchor   != Target::RightAnchor);
    static_assert(Target::ColorRed     != Target::ColorGreen);
    static_assert(Target::ColorGreen   != Target::ColorBlue);
    static_assert(Target::ColorBlue    != Target::ColorAlpha);
    static_assert(Target::Rotation     != Target::Unknown);
    static_assert(Target::FlipbookClip    != Target::Unknown);
    static_assert(Target::FlipbookPlaying != Target::FlipbookClip);

    // AnimationClip::LoopType distinctness
    static_assert(AnimationClip::LoopType::Stop  != AnimationClip::LoopType::Loop);
    static_assert(AnimationClip::LoopType::Loop  != AnimationClip::LoopType::Reset);
}

TEST_CASE("Keyframe fields are stable", "[api][animation][keyframe]") {
    SUCCEED();
}

TEST_CASE("AnimationTrack::Target enum values are stable", "[api][animation][track]") {
    SUCCEED();
}

TEST_CASE("AnimationTrack static arrays are stable", "[api][animation][track]") {
    SUCCEED();
}

TEST_CASE("AnimationTrack method signatures are stable", "[api][animation][track]") {
    AnimationTrack::Target (AnimationTrack::*getTarget)() const      = &AnimationTrack::GetTarget;
    AnimationTrack::KeyframeList& (AnimationTrack::*keyframes)()     = &AnimationTrack::Keyframes;
    Keyframe* (AnimationTrack::*getKf)(int)                          = &AnimationTrack::GetKeyframe;
    Keyframe& (AnimationTrack::*getOrCreate)(int)                    = &AnimationTrack::GetOrCreateKeyframe;
    void (AnimationTrack::*deleteByNo)(int)                          = &AnimationTrack::DeleteKeyframe;
    void (AnimationTrack::*deleteByPtr)(Keyframe*)                   = &AnimationTrack::DeleteKeyframe;
    float (AnimationTrack::*getVal)(float) const                     = &AnimationTrack::GetValueAtFrame;
    void (AnimationTrack::*sort)()                                   = &AnimationTrack::SortKeyframes;
    (void)getTarget; (void)keyframes; (void)getKf; (void)getOrCreate;
    (void)deleteByNo; (void)deleteByPtr; (void)getVal; (void)sort;
    SUCCEED();
}

TEST_CASE("AnimationClip fields and methods are stable", "[api][animation][clip]") {
    int (AnimationClip::*frameCount)() const = &AnimationClip::FrameCount;
    (void)frameCount;
    SUCCEED();
}

TEST_CASE("AnimationMarker fields are stable", "[api][animation][event]") {
    SUCCEED();
}
