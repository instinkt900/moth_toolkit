#include "moth_ui/animation/animation_clip.h"
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>

using namespace moth_ui;

TEST_CASE("AnimationClip default construction", "[animation_clip][initialization]") {
    AnimationClip clip;
    REQUIRE(clip.name.empty());
    REQUIRE(clip.startFrame == 0);
    REQUIRE(clip.endFrame == 0);
    REQUIRE(clip.fps == Catch::Approx(AnimationClip::kDefaultFPS));
    REQUIRE(clip.loopType == AnimationClip::LoopType::Stop);
}

TEST_CASE("AnimationClip kDefaultFPS is 30", "[animation_clip]") {
    REQUIRE(AnimationClip::kDefaultFPS == Catch::Approx(30.0f));
}

TEST_CASE("AnimationClip FrameCount is inclusive", "[animation_clip][frame_count]") {
    AnimationClip clip;

    clip.startFrame = 0;
    clip.endFrame = 9;
    REQUIRE(clip.FrameCount() == 10);

    clip.startFrame = 5;
    clip.endFrame = 5;
    REQUIRE(clip.FrameCount() == 1);

    clip.startFrame = 3;
    clip.endFrame = 13;
    REQUIRE(clip.FrameCount() == 11);
}

TEST_CASE("AnimationClip equality operators", "[animation_clip][operators]") {
    AnimationClip a;
    a.name = "run";
    a.startFrame = 0;
    a.endFrame = 10;
    a.fps = 30.0f;
    a.loopType = AnimationClip::LoopType::Loop;

    AnimationClip b = a;
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);

    b.endFrame = 11;
    REQUIRE(a != b);
    REQUIRE_FALSE(a == b);
}

TEST_CASE("AnimationClip inequality detects each field", "[animation_clip][operators]") {
    AnimationClip base;
    base.name = "idle";
    base.startFrame = 0;
    base.endFrame = 10;

    AnimationClip diffName = base;
    diffName.name = "walk";
    REQUIRE(base != diffName);

    AnimationClip diffStart = base;
    diffStart.startFrame = 1;
    REQUIRE(base != diffStart);

    AnimationClip diffFps = base;
    diffFps.fps = 24.0f;
    REQUIRE(base != diffFps);

    AnimationClip diffLoop = base;
    diffLoop.loopType = AnimationClip::LoopType::Reset;
    REQUIRE(base != diffLoop);
}

TEST_CASE("AnimationClip copy construction", "[animation_clip][copy]") {
    AnimationClip original;
    original.name = "jump";
    original.startFrame = 5;
    original.endFrame = 15;
    original.fps = 24.0f;
    original.loopType = AnimationClip::LoopType::Reset;

    AnimationClip copy = original;
    REQUIRE(copy == original);
}

TEST_CASE("AnimationClip JSON round-trip", "[animation_clip][json]") {
    AnimationClip clip;
    clip.name = "walk";
    clip.startFrame = 5;
    clip.endFrame = 20;
    clip.fps = 24.0f;
    clip.loopType = AnimationClip::LoopType::Loop;

    nlohmann::json j = clip;
    AnimationClip restored = j.get<AnimationClip>();
    REQUIRE(restored == clip);
}

TEST_CASE("AnimationClip LoopType JSON round-trip", "[animation_clip][json]") {
    for (auto loopType : { AnimationClip::LoopType::Stop,
                           AnimationClip::LoopType::Loop,
                           AnimationClip::LoopType::Reset }) {
        AnimationClip clip;
        clip.loopType = loopType;
        nlohmann::json j = clip;
        AnimationClip restored = j.get<AnimationClip>();
        REQUIRE(restored.loopType == loopType);
    }
}

TEST_CASE("AnimationClip FrameCount returns negative when startFrame > endFrame", "[animation_clip]") {
    AnimationClip clip;
    clip.startFrame = 10;
    clip.endFrame = 5;
    REQUIRE(clip.FrameCount() == -4);
}
