#include "moth_ui/animation/animation_clip.h"
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>

using namespace moth_ui;

TEST_CASE("AnimationClip default construction", "[animation_clip][initialization]") {
    AnimationClip clip;
    REQUIRE(clip.m_name.empty());
    REQUIRE(clip.m_startFrame == 0);
    REQUIRE(clip.m_endFrame == 0);
    REQUIRE(clip.m_fps == Catch::Approx(AnimationClip::DefaultFPS));
    REQUIRE(clip.m_loopType == AnimationClip::LoopType::Stop);
}

TEST_CASE("AnimationClip DefaultFPS is 30", "[animation_clip]") {
    REQUIRE(AnimationClip::DefaultFPS == Catch::Approx(30.0f));
}

TEST_CASE("AnimationClip FrameCount is inclusive", "[animation_clip][frame_count]") {
    AnimationClip clip;

    clip.m_startFrame = 0;
    clip.m_endFrame = 9;
    REQUIRE(clip.FrameCount() == 10);

    clip.m_startFrame = 5;
    clip.m_endFrame = 5;
    REQUIRE(clip.FrameCount() == 1);

    clip.m_startFrame = 3;
    clip.m_endFrame = 13;
    REQUIRE(clip.FrameCount() == 11);
}

TEST_CASE("AnimationClip equality operators", "[animation_clip][operators]") {
    AnimationClip a;
    a.m_name = "run";
    a.m_startFrame = 0;
    a.m_endFrame = 10;
    a.m_fps = 30.0f;
    a.m_loopType = AnimationClip::LoopType::Loop;

    AnimationClip b = a;
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);

    b.m_endFrame = 11;
    REQUIRE(a != b);
    REQUIRE_FALSE(a == b);
}

TEST_CASE("AnimationClip inequality detects each field", "[animation_clip][operators]") {
    AnimationClip base;
    base.m_name = "idle";
    base.m_startFrame = 0;
    base.m_endFrame = 10;

    AnimationClip diffName = base;
    diffName.m_name = "walk";
    REQUIRE(base != diffName);

    AnimationClip diffStart = base;
    diffStart.m_startFrame = 1;
    REQUIRE(base != diffStart);

    AnimationClip diffFps = base;
    diffFps.m_fps = 24.0f;
    REQUIRE(base != diffFps);

    AnimationClip diffLoop = base;
    diffLoop.m_loopType = AnimationClip::LoopType::Reset;
    REQUIRE(base != diffLoop);
}

TEST_CASE("AnimationClip copy construction", "[animation_clip][copy]") {
    AnimationClip original;
    original.m_name = "jump";
    original.m_startFrame = 5;
    original.m_endFrame = 15;
    original.m_fps = 24.0f;
    original.m_loopType = AnimationClip::LoopType::Reset;

    AnimationClip copy = original;
    REQUIRE(copy == original);
}

TEST_CASE("AnimationClip JSON round-trip", "[animation_clip][json]") {
    AnimationClip clip;
    clip.m_name = "walk";
    clip.m_startFrame = 5;
    clip.m_endFrame = 20;
    clip.m_fps = 24.0f;
    clip.m_loopType = AnimationClip::LoopType::Loop;

    nlohmann::json j = clip;
    AnimationClip restored = j.get<AnimationClip>();
    REQUIRE(restored == clip);
}

TEST_CASE("AnimationClip LoopType JSON round-trip", "[animation_clip][json]") {
    for (auto loopType : { AnimationClip::LoopType::Stop,
                           AnimationClip::LoopType::Loop,
                           AnimationClip::LoopType::Reset }) {
        AnimationClip clip;
        clip.m_loopType = loopType;
        nlohmann::json j = clip;
        AnimationClip restored = j.get<AnimationClip>();
        REQUIRE(restored.m_loopType == loopType);
    }
}
