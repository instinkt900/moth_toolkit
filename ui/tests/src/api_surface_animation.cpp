// Pins the animation system types: Keyframe, AnimationTrack, AnimationClip, AnimationEvent.

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

    // Namespace-scope static_assert: evaluated at translation-unit level, no
    // function-scope ambiguity about whether the expression is a constant expression.
    using Target = AnimationTrack::Target;
    constexpr std::array expectedContinuous = {
        Target::TopOffset,    Target::BottomOffset, Target::LeftOffset,  Target::RightOffset,
        Target::TopAnchor,    Target::BottomAnchor, Target::LeftAnchor,  Target::RightAnchor,
        Target::ColorRed,     Target::ColorGreen,   Target::ColorBlue,   Target::ColorAlpha,
        Target::Rotation
    };
    static_assert(arrays_equal(AnimationTrack::ContinuousTargets, expectedContinuous),
                  "AnimationTrack::ContinuousTargets contents or order changed");
}

TEST_CASE("Keyframe fields are stable", "[api][animation][keyframe]") {
    static_assert(std::is_same_v<decltype(Keyframe::m_frame),     int>);
    static_assert(std::is_same_v<decltype(Keyframe::m_value),     float>);
    static_assert(std::is_same_v<decltype(Keyframe::m_interpType), InterpType>);
    SUCCEED();
}

TEST_CASE("AnimationTrack::Target enum values are stable", "[api][animation][track]") {
    using T = AnimationTrack::Target;
    // Verify each enumerator exists and is distinct from Unknown
    static_assert(T::Unknown      != T::Events);
    static_assert(T::TopOffset    != T::BottomOffset);
    static_assert(T::LeftOffset   != T::RightOffset);
    static_assert(T::TopAnchor    != T::BottomAnchor);
    static_assert(T::LeftAnchor   != T::RightAnchor);
    static_assert(T::ColorRed     != T::ColorGreen);
    static_assert(T::ColorGreen   != T::ColorBlue);
    static_assert(T::ColorBlue    != T::ColorAlpha);
    static_assert(T::Rotation     != T::Unknown);
    static_assert(T::FlipbookClip    != T::Unknown);
    static_assert(T::FlipbookPlaying != T::FlipbookClip);
    SUCCEED();
}

TEST_CASE("AnimationTrack static arrays are stable", "[api][animation][track]") {
    // ContinuousTargets: pinned at namespace scope above (arrays_equal, expectedContinuous).
    // DiscreteTargets covers FlipbookClip and FlipbookPlaying.
    static_assert(AnimationTrack::DiscreteTargets.size()   == 2);
    static_assert(AnimationTrack::DiscreteTargets[0] == AnimationTrack::Target::FlipbookClip);
    static_assert(AnimationTrack::DiscreteTargets[1] == AnimationTrack::Target::FlipbookPlaying);
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
    static_assert(std::is_same_v<decltype(AnimationClip::m_name),      std::string>);
    static_assert(std::is_same_v<decltype(AnimationClip::m_startFrame), int>);
    static_assert(std::is_same_v<decltype(AnimationClip::m_endFrame),   int>);
    static_assert(std::is_same_v<decltype(AnimationClip::m_fps),        float>);
    static_assert(std::is_same_v<decltype(AnimationClip::m_loopType),   AnimationClip::LoopType>);

    static_assert(AnimationClip::LoopType::Stop  != AnimationClip::LoopType::Loop);
    static_assert(AnimationClip::LoopType::Loop  != AnimationClip::LoopType::Reset);

    int (AnimationClip::*frameCount)() const = &AnimationClip::FrameCount;
    (void)frameCount;
    SUCCEED();
}

TEST_CASE("AnimationEvent fields are stable", "[api][animation][event]") {
    static_assert(std::is_same_v<decltype(AnimationEvent::m_frame), int>);
    static_assert(std::is_same_v<decltype(AnimationEvent::m_name),  std::string>);
    SUCCEED();
}
