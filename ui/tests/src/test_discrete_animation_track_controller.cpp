#include "moth_ui/animation/animation_track.h"
#include "moth_ui/animation/discrete_animation_track.h"
#include "moth_ui/animation/discrete_animation_track_controller.h"
#include <catch2/catch_all.hpp>
#include <string>
#include <vector>

using namespace moth_ui;

TEST_CASE("DiscreteAnimationTrackController fires callback on first SetFrame", "[discrete_controller]") {
    DiscreteAnimationTrack track(AnimationTarget::FlipbookClip);
    track.GetOrCreateKeyframe(0) = "idle";

    std::vector<std::string> calls;
    DiscreteAnimationTrackController ctrl(track, [&](std::string_view v) {
        calls.push_back(std::string(v));
    });

    ctrl.SetFrame(0.0f);
    REQUIRE(calls.size() == 1);
    REQUIRE(calls[0] == "idle");
}

TEST_CASE("DiscreteAnimationTrackController fires callback on value change", "[discrete_controller]") {
    DiscreteAnimationTrack track(AnimationTarget::FlipbookClip);
    track.GetOrCreateKeyframe(0) = "idle";
    track.GetOrCreateKeyframe(10) = "run";

    std::vector<std::string> calls;
    DiscreteAnimationTrackController ctrl(track, [&](std::string_view v) {
        calls.push_back(std::string(v));
    });

    ctrl.SetFrame(0.0f);  // fires "idle"
    ctrl.SetFrame(5.0f);  // same value — no fire
    ctrl.SetFrame(10.0f); // fires "run"

    REQUIRE(calls.size() == 2);
    REQUIRE(calls[0] == "idle");
    REQUIRE(calls[1] == "run");
}

TEST_CASE("DiscreteAnimationTrackController does not re-fire on unchanged value", "[discrete_controller]") {
    DiscreteAnimationTrack track(AnimationTarget::FlipbookClip);
    track.GetOrCreateKeyframe(0) = "idle";

    int count = 0;
    DiscreteAnimationTrackController ctrl(track, [&](std::string_view) { ++count; });

    ctrl.SetFrame(0.0f);
    ctrl.SetFrame(1.0f);
    ctrl.SetFrame(2.0f);
    REQUIRE(count == 1);
}

TEST_CASE("DiscreteAnimationTrackController Reset forces re-fire on next SetFrame", "[discrete_controller]") {
    DiscreteAnimationTrack track(AnimationTarget::FlipbookClip);
    track.GetOrCreateKeyframe(0) = "idle";

    int count = 0;
    DiscreteAnimationTrackController ctrl(track, [&](std::string_view) { ++count; });

    ctrl.SetFrame(0.0f);
    REQUIRE(count == 1);

    ctrl.Reset();
    ctrl.SetFrame(0.0f); // same value, but Reset() cleared initialized flag
    REQUIRE(count == 2);
}

TEST_CASE("DiscreteAnimationTrackController fires empty string before first keyframe", "[discrete_controller]") {
    DiscreteAnimationTrack track(AnimationTarget::FlipbookClip);
    track.GetOrCreateKeyframe(10) = "idle";

    std::vector<std::string> calls;
    DiscreteAnimationTrackController ctrl(track, [&](std::string_view v) {
        calls.push_back(std::string(v));
    });

    ctrl.SetFrame(0.0f);  // before first keyframe — empty string
    REQUIRE(calls.size() == 1);
    REQUIRE(calls[0].empty());

    ctrl.SetFrame(10.0f); // reaches keyframe — fires "idle"
    REQUIRE(calls.size() == 2);
    REQUIRE(calls[1] == "idle");
}

TEST_CASE("DiscreteAnimationTrackController handles backward scrub with value change", "[discrete_controller]") {
    DiscreteAnimationTrack track(AnimationTarget::FlipbookClip);
    track.GetOrCreateKeyframe(0) = "idle";
    track.GetOrCreateKeyframe(10) = "run";

    std::vector<std::string> calls;
    DiscreteAnimationTrackController ctrl(track, [&](std::string_view v) {
        calls.push_back(std::string(v));
    });

    ctrl.SetFrame(10.0f); // fires "run" (first call)
    ctrl.SetFrame(0.0f);  // fires "idle" (changed)

    REQUIRE(calls.size() == 2);
    REQUIRE(calls[0] == "run");
    REQUIRE(calls[1] == "idle");
}
