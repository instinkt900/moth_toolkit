#include "moth/anim/anim_set.h"
#include "moth/anim/animator.h"
#include "moth/graphics/graphics/image.h"
#include "moth/graphics/graphics/spritesheet.h"
#include "moth/graphics/utils/rect.h"
#include "moth/graphics/utils/vector.h"

#include <catch2/catch_all.hpp>
#include <memory>
#include <string>
#include <vector>

using namespace moth::anim;
using namespace moth::gfx;

namespace {
    SpriteSheet::FrameEntry MakeFrame(int x, int y, int w, int h) {
        return { MakeRect(x, y, w, h), IntVec2{ 0, 0 } };
    }

    // Builds a SpriteSheet with 8 frames (8x8 each) and the given clips.
    std::shared_ptr<SpriteSheet> MakeSheet(std::vector<SpriteSheet::ClipEntry> clips, int numFrames = 8) {
        std::vector<SpriteSheet::FrameEntry> frames;
        frames.reserve(static_cast<size_t>(numFrames));
        for (int i = 0; i < numFrames; ++i) {
            frames.push_back(MakeFrame(i * 8, 0, 8, 8));
        }
        return std::make_shared<SpriteSheet>(Image{}, std::move(frames), std::move(clips));
    }

    SpriteSheet::ClipEntry Clip(std::string name,
                                std::vector<SpriteSheet::ClipFrame> steps,
                                SpriteSheet::LoopType loop = SpriteSheet::LoopType::Loop) {
        return { std::move(name), { std::move(steps), loop } };
    }

    AnimSet MakeSet(std::vector<StateSpec> states,
                    std::vector<TransitionSpec> transitions = {},
                    std::string initial = "idle") {
        AnimSet set;
        set.initial = std::move(initial);
        set.states = std::move(states);
        set.transitions = std::move(transitions);
        return set;
    }
}

TEST_CASE("Animator enters the initial state and plays its clip", "[animator]") {
    auto sheet = MakeSheet({ Clip("idle", { { 0, 100 } }) }, 1);
    Animator animator(sheet, MakeSet({ { "idle", "idle", std::nullopt } }));

    REQUIRE(animator.GetCurrentState() == "idle");
    REQUIRE(animator.GetSprite().GetCurrentClipName() == "idle");
    REQUIRE(animator.GetSprite().IsPlaying());
}

TEST_CASE("TransitionTo performs an instant cut and fires state callbacks", "[animator]") {
    auto sheet = MakeSheet({
        Clip("idle", { { 0, 100 } }),
        Clip("run", { { 1, 100 } }),
    }, 2);
    Animator animator(sheet, MakeSet({
        { "idle", "idle", std::nullopt },
        { "run", "run", std::nullopt },
    }));

    std::vector<std::string> entered;
    std::vector<std::string> exited;
    animator.OnStateEntered = [&](std::string_view s) { entered.emplace_back(s); };
    animator.OnStateExited = [&](std::string_view s) { exited.emplace_back(s); };

    animator.TransitionTo("run");

    REQUIRE(animator.GetCurrentState() == "run");
    REQUIRE(animator.GetSprite().GetCurrentClipName() == "run");
    REQUIRE(entered == std::vector<std::string>{ "run" });
    REQUIRE(exited == std::vector<std::string>{ "idle" });
}

TEST_CASE("A transition clip plays once before entering the target state", "[animator][transition]") {
    auto sheet = MakeSheet({
        Clip("idle", { { 0, 100 } }),
        Clip("run", { { 1, 100 } }),
        Clip("run_start", { { 2, 100 } }, SpriteSheet::LoopType::Stop),
    }, 3);
    Animator animator(sheet, MakeSet(
        { { "idle", "idle", std::nullopt }, { "run", "run", std::nullopt } },
        { { "idle.run", "idle", "run", "run_start" } }
    ));

    int started = 0;
    int completed = 0;
    animator.OnTransitionStarted = [&](std::string_view) { ++started; };
    animator.OnTransitionCompleted = [&](std::string_view id) {
        ++completed;
        REQUIRE(id == "idle.run");
    };

    animator.TransitionTo("run");

    // Still in the source state while the transition clip plays.
    REQUIRE(animator.GetCurrentState() == "idle");
    REQUIRE(animator.GetSprite().GetCurrentClipName() == "run_start");
    REQUIRE(started == 1);

    animator.Update(200);  // completes the one-shot transition clip

    REQUIRE(animator.GetCurrentState() == "run");
    REQUIRE(animator.GetSprite().GetCurrentClipName() == "run");
    REQUIRE(completed == 1);
}

TEST_CASE("onComplete chains one-shot states automatically", "[animator][oncomplete]") {
    auto sheet = MakeSheet({
        Clip("jump", { { 0, 100 } }, SpriteSheet::LoopType::Stop),
        Clip("landing", { { 1, 100 } }, SpriteSheet::LoopType::Stop),
        Clip("idle", { { 2, 100 } }),
    }, 3);
    Animator animator(sheet, MakeSet({
        { "jump", "jump", "landing" },
        { "landing", "landing", "idle" },
        { "idle", "idle", std::nullopt },
    }));

    std::vector<std::string> entered;
    animator.OnStateEntered = [&](std::string_view s) { entered.emplace_back(s); };

    animator.TransitionTo("jump");
    REQUIRE(animator.GetCurrentState() == "jump");

    animator.Update(200);  // jump -> landing
    REQUIRE(animator.GetCurrentState() == "landing");

    animator.Update(200);  // landing -> idle
    REQUIRE(animator.GetCurrentState() == "idle");
    REQUIRE(entered == std::vector<std::string>{ "jump", "landing", "idle" });
}

TEST_CASE("A non-looping state without onComplete freezes in place", "[animator][oncomplete]") {
    auto sheet = MakeSheet({
        Clip("attack", { { 0, 100 } }, SpriteSheet::LoopType::Stop),
    }, 1);
    Animator animator(sheet, MakeSet({ { "attack", "attack", std::nullopt } }));

    animator.TransitionTo("attack");
    animator.Update(200);

    REQUIRE(animator.GetCurrentState() == "attack");
    REQUIRE_FALSE(animator.GetSprite().IsPlaying());
}

TEST_CASE("A new TransitionTo cuts an in-flight transition clip", "[animator][interrupt]") {
    auto sheet = MakeSheet({
        Clip("idle", { { 0, 100 } }),
        Clip("run", { { 1, 100 } }),
        Clip("walk", { { 2, 100 } }),
        Clip("run_start", { { 3, 100 } }, SpriteSheet::LoopType::Stop),
    }, 4);
    Animator animator(sheet, MakeSet(
        { { "idle", "idle", std::nullopt }, { "run", "run", std::nullopt }, { "walk", "walk", std::nullopt } },
        { { "idle.run", "idle", "run", "run_start" } }
    ));

    animator.TransitionTo("run");   // begins the run_start transition clip
    animator.TransitionTo("walk");  // cuts it and enters walk immediately

    REQUIRE(animator.GetCurrentState() == "walk");
    REQUIRE(animator.GetSprite().GetCurrentClipName() == "walk");
}

TEST_CASE("TransitionTo an unknown state is a no-op with a warning", "[animator]") {
    auto sheet = MakeSheet({ Clip("idle", { { 0, 100 } }) }, 1);
    Animator animator(sheet, MakeSet({ { "idle", "idle", std::nullopt } }));

    animator.TransitionTo("nope");
    REQUIRE(animator.GetCurrentState() == "idle");
}

TEST_CASE("Clip-level callbacks are forwarded from the sprite", "[animator][callback]") {
    auto sheet = MakeSheet({
        Clip("anim", { { 0, 100 }, { 1, 100 } }, SpriteSheet::LoopType::Loop),
    }, 2);
    Animator animator(sheet, MakeSet({ { "anim", "anim", std::nullopt } }, {}, "anim"));

    int looped = 0;
    animator.OnClipLooped = [&](std::string_view name) {
        ++looped;
        REQUIRE(name == "anim");
    };

    animator.Update(250);  // wraps once
    REQUIRE(looped == 1);
}

TEST_CASE("The Animator delegates flip and speed to the sprite", "[animator][sprite]") {
    auto sheet = MakeSheet({ Clip("idle", { { 0, 100 } }) }, 1);
    Animator animator(sheet, MakeSet({ { "idle", "idle", std::nullopt } }));

    animator.GetSprite().SetFlipX(true);
    REQUIRE(animator.GetSprite().GetFlipX());

    animator.GetSprite().SetSpeed(2.0f);
    REQUIRE(animator.GetSprite().GetSpeed() == 2.0f);
}
