#include "moth_ui/animation/animation_track.h"
#include "moth_ui/animation/keyframe.h"
#include <catch2/catch_all.hpp>
#include <memory>

using namespace moth_ui;

TEST_CASE("AnimationTrack target construction", "[animation_track][initialization]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    REQUIRE(track.GetTarget() == AnimationTrack::Target::TopOffset);
    REQUIRE(track.Keyframes().empty());
}

TEST_CASE("AnimationTrack initial value construction", "[animation_track][initialization]") {
    AnimationTrack track(AnimationTrack::Target::ColorAlpha, 0.5f);
    REQUIRE(track.GetTarget() == AnimationTrack::Target::ColorAlpha);
    REQUIRE(track.Keyframes().size() == 1);
    REQUIRE(track.Keyframes()[0]->m_frame == 0);
    REQUIRE(track.Keyframes()[0]->m_value == Catch::Approx(0.5f));
}

TEST_CASE("AnimationTrack ContinuousTargets has 13 entries", "[animation_track]") {
    REQUIRE(AnimationTrack::ContinuousTargets.size() == 13);
}

TEST_CASE("AnimationTrack ContinuousTargets layout offsets and anchors", "[animation_track]") {
    auto& ct = AnimationTrack::ContinuousTargets;
    REQUIRE(ct[0] == AnimationTrack::Target::TopOffset);
    REQUIRE(ct[1] == AnimationTrack::Target::BottomOffset);
    REQUIRE(ct[2] == AnimationTrack::Target::LeftOffset);
    REQUIRE(ct[3] == AnimationTrack::Target::RightOffset);
    REQUIRE(ct[4] == AnimationTrack::Target::TopAnchor);
    REQUIRE(ct[5] == AnimationTrack::Target::BottomAnchor);
    REQUIRE(ct[6] == AnimationTrack::Target::LeftAnchor);
    REQUIRE(ct[7] == AnimationTrack::Target::RightAnchor);
}

TEST_CASE("AnimationTrack ContinuousTargets colour channels", "[animation_track]") {
    auto& ct = AnimationTrack::ContinuousTargets;
    REQUIRE(ct[8]  == AnimationTrack::Target::ColorRed);
    REQUIRE(ct[9]  == AnimationTrack::Target::ColorGreen);
    REQUIRE(ct[10] == AnimationTrack::Target::ColorBlue);
    REQUIRE(ct[11] == AnimationTrack::Target::ColorAlpha);
    REQUIRE(ct[12] == AnimationTrack::Target::Rotation);
}

TEST_CASE("AnimationTrack GetOrCreateKeyframe inserts sorted", "[animation_track][keyframes]") {
    AnimationTrack track(AnimationTrack::Target::LeftOffset);

    track.GetOrCreateKeyframe(10).m_value = 10.0f;
    track.GetOrCreateKeyframe(0).m_value  = 0.0f;
    track.GetOrCreateKeyframe(5).m_value  = 5.0f;

    auto& kfs = track.Keyframes();
    REQUIRE(kfs.size() == 3);
    REQUIRE(kfs[0]->m_frame == 0);
    REQUIRE(kfs[1]->m_frame == 5);
    REQUIRE(kfs[2]->m_frame == 10);
}

TEST_CASE("AnimationTrack GetOrCreateKeyframe returns existing", "[animation_track][keyframes]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    track.GetOrCreateKeyframe(5).m_value = 42.0f;

    REQUIRE(track.Keyframes().size() == 1);
    auto& again = track.GetOrCreateKeyframe(5);
    REQUIRE(again.m_value == Catch::Approx(42.0f));
    REQUIRE(track.Keyframes().size() == 1);
}

TEST_CASE("AnimationTrack GetKeyframe finds by frame number", "[animation_track][keyframes]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    track.GetOrCreateKeyframe(3).m_value = 99.0f;

    auto* kf = track.GetKeyframe(3);
    REQUIRE(kf != nullptr);
    REQUIRE(kf->m_value == Catch::Approx(99.0f));
}

TEST_CASE("AnimationTrack GetKeyframe returns null for missing frame", "[animation_track][keyframes]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    track.GetOrCreateKeyframe(3);

    REQUIRE(track.GetKeyframe(99) == nullptr);
}

TEST_CASE("AnimationTrack DeleteKeyframe by frame number", "[animation_track][keyframes]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    track.GetOrCreateKeyframe(0);
    track.GetOrCreateKeyframe(10);
    REQUIRE(track.Keyframes().size() == 2);

    track.DeleteKeyframe(0);
    REQUIRE(track.Keyframes().size() == 1);
    REQUIRE(track.GetKeyframe(0)  == nullptr);
    REQUIRE(track.GetKeyframe(10) != nullptr);
}

TEST_CASE("AnimationTrack DeleteKeyframe by pointer", "[animation_track][keyframes]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    auto& kf = track.GetOrCreateKeyframe(5);
    REQUIRE(track.Keyframes().size() == 1);

    track.DeleteKeyframe(&kf);
    REQUIRE(track.Keyframes().empty());
}

TEST_CASE("AnimationTrack SortKeyframes reorders by frame", "[animation_track][sort]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    auto& kfs = track.Keyframes();
    kfs.push_back(std::make_shared<Keyframe>(10, 10.0f));
    kfs.push_back(std::make_shared<Keyframe>(0,  0.0f));
    kfs.push_back(std::make_shared<Keyframe>(5,  5.0f));

    track.SortKeyframes();

    REQUIRE(kfs[0]->m_frame == 0);
    REQUIRE(kfs[1]->m_frame == 5);
    REQUIRE(kfs[2]->m_frame == 10);
}

TEST_CASE("AnimationTrack GetValueAtFrame no keyframes returns 0", "[animation_track][value]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    REQUIRE(track.GetValueAtFrame(5.0f) == Catch::Approx(0.0f));
}

TEST_CASE("AnimationTrack GetValueAtFrame single keyframe", "[animation_track][value]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    track.GetOrCreateKeyframe(5).m_value = 42.0f;

    REQUIRE(track.GetValueAtFrame(0.0f)   == Catch::Approx(42.0f));
    REQUIRE(track.GetValueAtFrame(5.0f)   == Catch::Approx(42.0f));
    REQUIRE(track.GetValueAtFrame(100.0f) == Catch::Approx(42.0f));
}

TEST_CASE("AnimationTrack GetValueAtFrame linear interpolation between two keyframes", "[animation_track][value]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    track.GetOrCreateKeyframe(0).m_value  = 0.0f;
    track.GetOrCreateKeyframe(10).m_value = 10.0f;
    // Default interpType is Linear

    REQUIRE(track.GetValueAtFrame(0.0f)  == Catch::Approx(0.0f));
    REQUIRE(track.GetValueAtFrame(10.0f) == Catch::Approx(10.0f));
    REQUIRE(track.GetValueAtFrame(5.0f)  == Catch::Approx(5.0f));
    REQUIRE(track.GetValueAtFrame(2.5f)  == Catch::Approx(2.5f));
}

TEST_CASE("AnimationTrack GetValueAtFrame step interpolation", "[animation_track][value]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    auto& kf0 = track.GetOrCreateKeyframe(0);
    kf0.m_value = 0.0f;
    kf0.m_interpType = InterpType::Step;

    track.GetOrCreateKeyframe(10).m_value = 100.0f;

    // Step holds start value until we reach the next keyframe
    REQUIRE(track.GetValueAtFrame(5.0f) == Catch::Approx(0.0f));
}

TEST_CASE("AnimationTrack GetValueAtFrame holds last keyframe value beyond end", "[animation_track][value]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    track.GetOrCreateKeyframe(0).m_value  = 0.0f;
    track.GetOrCreateKeyframe(10).m_value = 50.0f;

    REQUIRE(track.GetValueAtFrame(20.0f) == Catch::Approx(50.0f));
}

TEST_CASE("AnimationTrack ForKeyframesOverFrames visits correct keyframes", "[animation_track][iteration]") {
    AnimationTrack track(AnimationTrack::Target::TopOffset);
    track.GetOrCreateKeyframe(0).m_value  = 0.0f;
    track.GetOrCreateKeyframe(5).m_value  = 5.0f;
    track.GetOrCreateKeyframe(10).m_value = 10.0f;

    std::vector<int> visited;
    track.ForKeyframesOverFrames(0.0f, 10.0f, [&](Keyframe const& kf) {
        visited.push_back(kf.m_frame);
    });

    // ForKeyframesOverFrames visits frames strictly after startFrame and up to/including endFrame
    REQUIRE(visited.size() == 2);
    REQUIRE(visited[0] == 5);
    REQUIRE(visited[1] == 10);
}

TEST_CASE("AnimationTrack copy construction deep-copies keyframes", "[animation_track][copy]") {
    AnimationTrack original(AnimationTrack::Target::TopOffset);
    original.GetOrCreateKeyframe(0).m_value = 7.0f;

    AnimationTrack copy = original;
    copy.GetOrCreateKeyframe(0).m_value = 99.0f;

    REQUIRE(original.GetKeyframe(0)->m_value == Catch::Approx(7.0f));
}
