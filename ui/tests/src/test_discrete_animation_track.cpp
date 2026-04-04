#include "moth_ui/animation/animation_track.h"
#include "moth_ui/animation/discrete_animation_track.h"
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>

using namespace moth_ui;

TEST_CASE("DiscreteAnimationTrack construction sets target and has no keyframes", "[discrete_track]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    REQUIRE(track.GetTarget() == AnimationTrack::Target::FlipbookClip);
    REQUIRE(track.Keyframes().empty());
}

TEST_CASE("DiscreteAnimationTrack GetValueAtFrame returns empty with no keyframes", "[discrete_track][value]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    REQUIRE(track.GetValueAtFrame(0).empty());
    REQUIRE(track.GetValueAtFrame(10).empty());
}

TEST_CASE("DiscreteAnimationTrack GetValueAtFrame returns last value at or before frame", "[discrete_track][value]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    track.GetOrCreateKeyframe(0) = "idle";
    track.GetOrCreateKeyframe(10) = "run";

    REQUIRE(track.GetValueAtFrame(0) == "idle");
    REQUIRE(track.GetValueAtFrame(5) == "idle");
    REQUIRE(track.GetValueAtFrame(9) == "idle");
    REQUIRE(track.GetValueAtFrame(10) == "run");
    REQUIRE(track.GetValueAtFrame(99) == "run");
}

TEST_CASE("DiscreteAnimationTrack GetValueAtFrame returns empty before first keyframe", "[discrete_track][value]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    track.GetOrCreateKeyframe(5) = "idle";

    REQUIRE(track.GetValueAtFrame(0).empty());
    REQUIRE(track.GetValueAtFrame(4).empty());
    REQUIRE(track.GetValueAtFrame(5) == "idle");
}

TEST_CASE("DiscreteAnimationTrack GetOrCreateKeyframe inserts sorted", "[discrete_track][keyframes]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    track.GetOrCreateKeyframe(10) = "c";
    track.GetOrCreateKeyframe(0) = "a";
    track.GetOrCreateKeyframe(5) = "b";

    auto const& kfs = track.Keyframes();
    REQUIRE(kfs.size() == 3);
    REQUIRE(kfs[0].first == 0);
    REQUIRE(kfs[1].first == 5);
    REQUIRE(kfs[2].first == 10);
}

TEST_CASE("DiscreteAnimationTrack GetOrCreateKeyframe returns existing keyframe", "[discrete_track][keyframes]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    track.GetOrCreateKeyframe(5) = "idle";

    REQUIRE(track.Keyframes().size() == 1);
    track.GetOrCreateKeyframe(5) = "run";
    REQUIRE(track.Keyframes().size() == 1);
    REQUIRE(track.GetValueAtFrame(5) == "run");
}

TEST_CASE("DiscreteAnimationTrack GetKeyframe finds by frame number", "[discrete_track][keyframes]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    track.GetOrCreateKeyframe(3) = "hit";

    auto* v = track.GetKeyframe(3);
    REQUIRE(v != nullptr);
    REQUIRE(*v == "hit");
}

TEST_CASE("DiscreteAnimationTrack GetKeyframe returns null for missing frame", "[discrete_track][keyframes]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    track.GetOrCreateKeyframe(3) = "hit";

    REQUIRE(track.GetKeyframe(99) == nullptr);
}

TEST_CASE("DiscreteAnimationTrack DeleteKeyframe removes the entry", "[discrete_track][keyframes]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    track.GetOrCreateKeyframe(0) = "a";
    track.GetOrCreateKeyframe(5) = "b";
    REQUIRE(track.Keyframes().size() == 2);

    track.DeleteKeyframe(0);
    REQUIRE(track.Keyframes().size() == 1);
    REQUIRE(track.GetKeyframe(0) == nullptr);
    REQUIRE(track.GetKeyframe(5) != nullptr);
}

TEST_CASE("DiscreteAnimationTrack DeleteKeyframe on nonexistent frame is a no-op", "[discrete_track][keyframes]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    track.GetOrCreateKeyframe(0) = "a";

    track.DeleteKeyframe(99);
    REQUIRE(track.Keyframes().size() == 1);
}

TEST_CASE("DiscreteAnimationTrack SortKeyframes reorders by ascending frame", "[discrete_track][sort]") {
    DiscreteAnimationTrack track(AnimationTrack::Target::FlipbookClip);
    track.Keyframes().push_back({ 10, "c" });
    track.Keyframes().push_back({ 0,  "a" });
    track.Keyframes().push_back({ 5,  "b" });

    track.SortKeyframes();

    auto const& kfs = track.Keyframes();
    REQUIRE(kfs[0].first == 0);
    REQUIRE(kfs[1].first == 5);
    REQUIRE(kfs[2].first == 10);
    REQUIRE(kfs[0].second == "a");
    REQUIRE(kfs[1].second == "b");
    REQUIRE(kfs[2].second == "c");
}

TEST_CASE("DiscreteAnimationTrack to_json/from_json roundtrip", "[discrete_track][serialization]") {
    DiscreteAnimationTrack original(AnimationTrack::Target::FlipbookClip);
    original.GetOrCreateKeyframe(0) = "idle";
    original.GetOrCreateKeyframe(10) = "run";

    nlohmann::json j;
    to_json(j, original);

    DiscreteAnimationTrack deserialized(AnimationTrack::Target::Unknown);
    from_json(j, deserialized);

    REQUIRE(deserialized.GetTarget() == AnimationTrack::Target::FlipbookClip);
    REQUIRE(deserialized.Keyframes().size() == 2);
    REQUIRE(deserialized.GetValueAtFrame(0) == "idle");
    REQUIRE(deserialized.GetValueAtFrame(10) == "run");
}

TEST_CASE("DiscreteAnimationTrack to_json/from_json preserves empty track", "[discrete_track][serialization]") {
    DiscreteAnimationTrack original(AnimationTrack::Target::FlipbookPlaying);

    nlohmann::json j;
    to_json(j, original);

    DiscreteAnimationTrack deserialized(AnimationTrack::Target::Unknown);
    from_json(j, deserialized);

    REQUIRE(deserialized.GetTarget() == AnimationTrack::Target::FlipbookPlaying);
    REQUIRE(deserialized.Keyframes().empty());
}
