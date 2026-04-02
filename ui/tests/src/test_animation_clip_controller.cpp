#include "mock_context.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/animation/animation_event.h"
#include "moth_ui/events/event_animation.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/nodes/group.h"
#include <catch2/catch_all.hpp>
#include <memory>
#include <string>
#include <vector>

using namespace moth_ui;

namespace {
    // 10 fps so each 100 ms tick == 1 frame.
    constexpr int kClipEndFrame = 10;
    constexpr float kTestFps = 10.0f;
    constexpr uint32_t kOneTick = 100; // ms

    // Creates a Group backed by a LayoutEntityGroup so CheckEvents can access
    // the event list.
    struct AnimFixture {
        MockContext mc;
        std::shared_ptr<Layout> layout;
        std::shared_ptr<AnimationClip> clip;
        std::shared_ptr<Group> group;

        std::vector<std::string> startedClips;
        std::vector<std::string> stoppedClips;
        std::vector<std::string> markerNames;

        explicit AnimFixture(int startFrame = 0, int endFrame = kClipEndFrame,
                             AnimationClip::LoopType loopType = AnimationClip::LoopType::Stop,
                             float fps = kTestFps) {
            layout = std::make_shared<Layout>();

            clip = std::make_shared<AnimationClip>();
            clip->m_name = "test";
            clip->m_startFrame = startFrame;
            clip->m_endFrame = endFrame;
            clip->m_fps = fps;
            clip->m_loopType = loopType;
            layout->m_clips.push_back(clip);

            group = std::make_shared<Group>(mc.context, layout);
            group->SetEventHandler([this](Node* /*node*/, Event const& e) -> bool {
                if (auto const* ev = event_cast<EventAnimationStarted>(e)) {
                    startedClips.push_back(ev->GetClipName());
                } else if (auto const* ev = event_cast<EventAnimationStopped>(e)) {
                    stoppedClips.push_back(ev->GetClipName());
                } else if (auto const* ev = event_cast<EventAnimation>(e)) {
                    markerNames.push_back(ev->GetName());
                }
                return false;
            });
        }

        void addMarker(int frame, std::string const& name) {
            layout->m_events.push_back(std::make_unique<AnimationEvent>(frame, name));
        }

        void start() { group->SetAnimation("test"); }

        // Advance the group by the given number of milliseconds.
        void tick(uint32_t ms = kOneTick) { group->Update(ms); }
    };

    constexpr int kMidFrame = 5;          // frame inside [0..kClipEndFrame]
    constexpr int kMarkerFrameA = 3;      // first marker in ordering tests
    constexpr int kMarkerFrameB = 7;      // second marker in ordering tests
    constexpr int kOvershootTicks = 15;   // > one full loop (10 frames)
} // namespace

TEST_CASE("EventAnimationStarted fires immediately when SetAnimation is called", "[animation][clip_controller]") {
    AnimFixture f;
    f.start();
    REQUIRE(f.startedClips.size() == 1);
    REQUIRE(f.startedClips[0] == "test");
}

TEST_CASE("No events fire before SetAnimation is called", "[animation][clip_controller]") {
    AnimFixture f;
    f.tick(kOneTick * kClipEndFrame); // advance a full clip's worth with no clip set
    REQUIRE(f.startedClips.empty());
    REQUIRE(f.stoppedClips.empty());
    REQUIRE(f.markerNames.empty());
}

TEST_CASE("EventAnimationStopped fires when Stop clip reaches its end frame", "[animation][clip_controller]") {
    AnimFixture f; // [0..kClipEndFrame], Stop, kTestFps
    f.start();

    for (int i = 0; i < kClipEndFrame - 1; ++i) {
        f.tick();
    }
    REQUIRE(f.stoppedClips.empty()); // not yet at end frame

    f.tick(); // reaches end frame → clip ends
    REQUIRE(f.stoppedClips.size() == 1);
    REQUIRE(f.stoppedClips[0] == "test");
}

TEST_CASE("EventAnimationStopped fires when Reset clip reaches its end frame", "[animation][clip_controller]") {
    AnimFixture f(0, kClipEndFrame, AnimationClip::LoopType::Reset);
    f.start();

    for (int i = 0; i < kClipEndFrame - 1; ++i) {
        f.tick();
    }
    REQUIRE(f.stoppedClips.empty());

    f.tick();
    REQUIRE(f.stoppedClips.size() == 1);
    REQUIRE(f.stoppedClips[0] == "test");
}

TEST_CASE("EventAnimationStopped does not fire for Loop clips", "[animation][clip_controller]") {
    AnimFixture f(0, kClipEndFrame, AnimationClip::LoopType::Loop);
    f.start();

    // Play through more than one full loop and check that it never stops.
    for (int i = 0; i < kOvershootTicks; ++i) {
        f.tick();
    }
    REQUIRE(f.stoppedClips.empty());
}

TEST_CASE("EventAnimation marker fires at its exact frame", "[animation][clip_controller]") {
    AnimFixture f;
    f.addMarker(kMidFrame, "midpoint");
    f.start();

    // Advance to one frame before the marker — it must not have fired yet.
    for (int i = 0; i < kMidFrame - 1; ++i) {
        f.tick();
    }
    REQUIRE(f.markerNames.empty());

    // One more tick reaches the marker frame → fires.
    f.tick();
    REQUIRE(f.markerNames.size() == 1);
    REQUIRE(f.markerNames[0] == "midpoint");
}

TEST_CASE("EventAnimation marker does not fire before its frame", "[animation][clip_controller]") {
    AnimFixture f;
    f.addMarker(kMidFrame, "event");
    f.start();

    for (int i = 0; i < kMidFrame - 1; ++i) {
        f.tick();
    }
    REQUIRE(f.markerNames.empty());
}

TEST_CASE("Multiple markers fire in frame order", "[animation][clip_controller]") {
    AnimFixture f;
    f.addMarker(kMarkerFrameA, "alpha");
    f.addMarker(kMarkerFrameB, "beta");
    f.start();

    for (int i = 0; i < kMarkerFrameA - 1; ++i) {
        f.tick(); // frames before alpha
    }
    REQUIRE(f.markerNames.empty());

    f.tick(); // frame kMarkerFrameA → alpha
    REQUIRE(f.markerNames.size() == 1);
    REQUIRE(f.markerNames[0] == "alpha");

    for (int i = 0; i < kMarkerFrameB - kMarkerFrameA - 1; ++i) {
        f.tick(); // frames between alpha and beta
    }
    REQUIRE(f.markerNames.size() == 1);

    f.tick(); // frame kMarkerFrameB → beta
    REQUIRE(f.markerNames.size() == 2);
    REQUIRE(f.markerNames[1] == "beta");
}

TEST_CASE("EventAnimation marker fires on every loop iteration for Loop clips", "[animation][clip_controller]") {
    AnimFixture f(0, kClipEndFrame, AnimationClip::LoopType::Loop);
    f.addMarker(kMidFrame, "ping");
    f.start();

    // First pass — advance to the marker frame.
    for (int i = 0; i < kMidFrame; ++i) {
        f.tick();
    }
    REQUIRE(f.markerNames.size() == 1);

    // Advance past the loop boundary and into the second pass through the marker.
    // Total kMidFrame + kClipEndFrame ticks from start → marker in the second loop.
    for (int i = 0; i < kClipEndFrame; ++i) {
        f.tick();
    }
    REQUIRE(f.markerNames.size() == 2);
}

TEST_CASE("No events fire after Stop clip ends", "[animation][clip_controller]") {
    AnimFixture f;
    f.addMarker(kMarkerFrameA, "early");
    f.start();

    // Play the full clip to completion.
    for (int i = 0; i < kClipEndFrame; ++i) {
        f.tick();
    }
    REQUIRE(f.markerNames.size() == 1); // fired exactly once at kMarkerFrameA
    REQUIRE(f.stoppedClips.size() == 1);

    // Additional updates after the clip has stopped must not fire new events.
    f.tick(kOneTick * kClipEndFrame);
    REQUIRE(f.markerNames.size() == 1);
    REQUIRE(f.stoppedClips.size() == 1);
}

// ---------------------------------------------------------------------------
// Tests — weak_ptr node references in animation events
// ---------------------------------------------------------------------------

TEST_CASE("EventAnimationStarted GetNode locks to the firing group", "[animation][clip_controller][weak_ptr]") {
    AnimFixture f;

    std::shared_ptr<Node> capturedNode;
    f.group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (auto const* e = event_cast<EventAnimationStarted>(ev)) {
            capturedNode = e->GetNode().lock();
        }
        return false;
    });

    f.start();

    REQUIRE(capturedNode != nullptr);
    REQUIRE(capturedNode.get() == f.group.get());
}

TEST_CASE("EventAnimationStopped GetNode locks to the firing group", "[animation][clip_controller][weak_ptr]") {
    AnimFixture f; // Stop loop type by default

    std::shared_ptr<Node> capturedNode;
    f.group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (auto const* e = event_cast<EventAnimationStopped>(ev)) {
            capturedNode = e->GetNode().lock();
        }
        return false;
    });

    f.start();
    for (int i = 0; i < kClipEndFrame; ++i) {
        f.tick();
    }

    REQUIRE(capturedNode != nullptr);
    REQUIRE(capturedNode.get() == f.group.get());
}

TEST_CASE("EventAnimation GetNode locks to the firing group", "[animation][clip_controller][weak_ptr]") {
    AnimFixture f;
    f.addMarker(kMidFrame, "ping");

    std::shared_ptr<Node> capturedNode;
    f.group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (auto const* e = event_cast<EventAnimation>(ev)) {
            capturedNode = e->GetNode().lock();
        }
        return false;
    });

    f.start();
    for (int i = 0; i < kMidFrame; ++i) {
        f.tick();
    }

    REQUIRE(capturedNode != nullptr);
    REQUIRE(capturedNode.get() == f.group.get());
}
