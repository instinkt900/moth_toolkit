#include "mock_context.h"
#include "moth_ui/animation/animation_track.h"
#include "moth_ui/events/event_flipbook.h"
#include "moth_ui/graphics/iflipbook.h"
#include "moth_ui/graphics/iimage.h"
#include "moth_ui/iflipbook_factory.h"
#include "moth_ui/layout/layout_entity_flipbook.h"
#include "moth_ui/layout/layout_rect.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/nodes/node_flipbook.h"
#include "moth_ui/utils/rect.h"
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

using namespace moth_ui;

// ---------------------------------------------------------------------------
// Test doubles
// ---------------------------------------------------------------------------

class MockImage : public IImage {
public:
    int GetWidth() const override { return 64; }
    int GetHeight() const override { return 64; }
    IntVec2 GetDimensions() const override { return { 64, 64 }; }
};

class MockFlipbook : public IFlipbook {
public:
    mutable MockImage image;
    std::vector<FrameDesc> frames;
    std::map<std::string, ClipDesc, std::less<>> clips;

    IImage const& GetImage() const override { return image; }

    int GetFrameCount() const override { return static_cast<int>(frames.size()); }

    bool GetFrameDesc(int index, FrameDesc& outDesc) const override {
        if (index < 0 || index >= static_cast<int>(frames.size())) { return false; }
        outDesc = frames[index];
        return true;
    }

    int GetClipCount() const override { return static_cast<int>(clips.size()); }

    std::string_view GetClipName(int index) const override {
        int i = 0;
        for (auto const& [name, _] : clips) {
            if (i++ == index) { return name; }
        }
        return {};
    }

    bool GetClipDesc(std::string_view name, ClipDesc& outDesc) const override {
        auto it = clips.find(name);
        if (it == clips.end()) { return false; }
        outDesc = it->second;
        return true;
    }
};

class MockFlipbookFactory : public IFlipbookFactory {
public:
    MockFlipbook* nextFlipbook = nullptr; // non-owning pointer to the flipbook to return
    int getFlipbookCalls = 0;
    std::filesystem::path lastRequestedPath;

    std::unique_ptr<IFlipbook> GetFlipbook(std::filesystem::path const& path) override {
        ++getFlipbookCalls;
        lastRequestedPath = path;
        if (nextFlipbook == nullptr) { return nullptr; }
        // Build a new MockFlipbook cloned from nextFlipbook for each call.
        auto fb = std::make_unique<MockFlipbook>();
        *fb = *nextFlipbook;
        return fb;
    }
};

struct FlipbookTestContext {
    MockRenderer renderer;
    MockImageFactory imageFactory;
    MockFontFactory fontFactory;
    MockFlipbookFactory flipbookFactory;
    moth_ui::Context context{ &imageFactory, &fontFactory, &renderer, &flipbookFactory };
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Build a ClipDesc from a list of atlas frame indices, each held for durationMs.
static IFlipbook::ClipDesc MakeClip(std::initializer_list<int> frameIndices,
                                    int durationMs,
                                    IFlipbook::LoopType loop) {
    IFlipbook::ClipDesc clip;
    clip.loop = loop;
    for (int idx : frameIndices) {
        clip.frames.push_back({ idx, durationMs });
    }
    return clip;
}

// Build a ClipDesc from per-frame (frameIndex, durationMs) pairs.
static IFlipbook::ClipDesc MakeClip(std::initializer_list<std::pair<int, int>> frameSteps,
                                    IFlipbook::LoopType loop) {
    IFlipbook::ClipDesc clip;
    clip.loop = loop;
    for (auto const& [frameIndex, durationMs] : frameSteps) {
        clip.frames.push_back({ frameIndex, durationMs });
    }
    return clip;
}

// Build a MockFlipbook with 4 frames (16×16 each) and one clip "run" (frames 0–3,
// 84 ms per frame ≈ 12 FPS, Loop).
static MockFlipbook MakeSimpleFlipbook() {
    MockFlipbook fb;
    for (int i = 0; i < 4; ++i) {
        fb.frames.push_back({ MakeRect(i * 16, 0, 16, 16), { 8, 8 } });
    }
    fb.clips["run"] = MakeClip({ 0, 1, 2, 3 }, 84, IFlipbook::LoopType::Loop);
    return fb;
}

// ---------------------------------------------------------------------------
// Tests — Load path forwarding
// ---------------------------------------------------------------------------

TEST_CASE("Load with null factory clears all flipbook state", "[flipbook][load]") {
    // Context has no flipbook factory — Load should leave the node fully cleared.
    MockContext mc;
    auto node = NodeFlipbook::Create(mc.context);

    // Pre-load a valid flipbook so there is state to clear.
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto primed = NodeFlipbook::Create(tc.context);
    primed->Load("dummy.flipbook.json");
    primed->SetClip("run");
    REQUIRE(primed->GetFlipbook() != nullptr);

    // Now call Load on a node whose context has no factory.
    node->Load("dummy.flipbook.json");
    REQUIRE(node->GetFlipbook() == nullptr);
    REQUIRE(node->GetCurrentClipName().empty());
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("Load with factory returning null clears all flipbook state", "[flipbook][load]") {
    FlipbookTestContext tc;
    // nextFlipbook is null — factory returns nullptr.
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("nonexistent.flipbook.json");

    REQUIRE(node->GetFlipbook() == nullptr);
    REQUIRE(node->GetCurrentClipName().empty());
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("Load clears previous flipbook when factory returns null", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    REQUIRE(node->GetFlipbook() != nullptr);

    // Now make the factory return null and reload.
    tc.flipbookFactory.nextFlipbook = nullptr;
    node->Load("dummy.flipbook.json");

    REQUIRE(node->GetFlipbook() == nullptr);
    REQUIRE(node->GetCurrentClipName().empty());
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("Load with zero frames clears flipbook", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb; // no frames added
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");

    REQUIRE(node->GetFlipbook() == nullptr);
    REQUIRE(node->GetCurrentClipName().empty());
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("Load forwards path to flipbook factory", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = NodeFlipbook::Create(tc.context);
    node->Load(std::filesystem::path{ "dummy.flipbook.json" });

    REQUIRE(tc.flipbookFactory.getFlipbookCalls == 1);
    REQUIRE(tc.flipbookFactory.lastRequestedPath == std::filesystem::path{ "dummy.flipbook.json" });
}

TEST_CASE("Load resets playback state before loading", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);
    node->Update(84); // advance one step

    REQUIRE(node->GetCurrentFrame() == 1);

    // Reload — frame, time, and playing should all reset.
    node->Load("dummy.flipbook.json");
    REQUIRE(node->GetCurrentFrame() == 0);
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("SetClip after Load activates the named clip", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");

    REQUIRE(node->GetCurrentClipName() == "run");
    REQUIRE(node->GetCurrentFrame() == 0); // always starts at first step
}

TEST_CASE("Load with no clip set leaves clip unset", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    // No SetClip call — clip stays empty.

    REQUIRE(node->GetCurrentClipName().empty());
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("Discrete track FlipbookPlaying=1 at frame 0 starts playback on instantiate", "[flipbook][load][discrete]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto entity = std::make_shared<LayoutEntityFlipbook>(MakeDefaultLayoutRect(), "dummy.flipbook.json");
    entity->m_discreteTracks.at(AnimationTarget::FlipbookClip).GetOrCreateKeyframe(0) = "run";
    entity->m_discreteTracks.at(AnimationTarget::FlipbookPlaying).GetOrCreateKeyframe(0) = "1";

    auto node = std::dynamic_pointer_cast<NodeFlipbook>(entity->Instantiate(tc.context));

    REQUIRE(node->IsPlaying());
    REQUIRE(node->GetCurrentClipName() == "run");
}

TEST_CASE("Discrete track FlipbookPlaying=0 at frame 0 does not start playback on instantiate", "[flipbook][load][discrete]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto entity = std::make_shared<LayoutEntityFlipbook>(MakeDefaultLayoutRect(), "dummy.flipbook.json");
    entity->m_discreteTracks.at(AnimationTarget::FlipbookClip).GetOrCreateKeyframe(0) = "run";
    // FlipbookPlaying defaults to "0" at frame 0 from InitDiscreteFlipbookTracks.

    auto node = std::dynamic_pointer_cast<NodeFlipbook>(entity->Instantiate(tc.context));

    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentClipName() == "run");
}

TEST_CASE("Discrete track FlipbookPlaying=1 with no clip does not start playback", "[flipbook][load][discrete]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto entity = std::make_shared<LayoutEntityFlipbook>(MakeDefaultLayoutRect(), "dummy.flipbook.json");
    entity->m_discreteTracks.at(AnimationTarget::FlipbookPlaying).GetOrCreateKeyframe(0) = "1";
    // No clip name set in FlipbookClip track.

    auto node = std::dynamic_pointer_cast<NodeFlipbook>(entity->Instantiate(tc.context));

    REQUIRE_FALSE(node->IsPlaying());
}

// ---------------------------------------------------------------------------
// Tests — no flipbook loaded
// ---------------------------------------------------------------------------

TEST_CASE("NodeFlipbook default state: no clip, not playing, frame 0", "[flipbook]") {
    FlipbookTestContext tc;
    auto node = NodeFlipbook::Create(tc.context);

    REQUIRE(node->GetFlipbook() == nullptr);
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
    REQUIRE(node->GetCurrentClipName().empty());
}

TEST_CASE("NodeFlipbook Update without flipbook does not crash", "[flipbook]") {
    FlipbookTestContext tc;
    auto node = NodeFlipbook::Create(tc.context);
    node->Update(1000);
    REQUIRE(node->GetCurrentFrame() == 0);
}

// ---------------------------------------------------------------------------
// Tests — SetClip
// ---------------------------------------------------------------------------

TEST_CASE("SetClip with valid name resets to first step of the clip", "[flipbook][clip]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    for (int i = 0; i < 6; ++i) {
        fb.frames.push_back({ MakeRect(i * 16, 0, 16, 16), { 8, 8 } });
    }
    fb.clips["idle"] = MakeClip({ 0, 1, 2 }, 100, IFlipbook::LoopType::Loop);
    fb.clips["run"]  = MakeClip({ 3, 4, 5 }, 84,  IFlipbook::LoopType::Loop);

    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");

    node->SetClip("run");
    REQUIRE(node->GetCurrentClipName() == "run");
    REQUIRE(node->GetCurrentFrame() == 0); // always starts at first step of the clip
}

TEST_CASE("SetClip with invalid name clears clip", "[flipbook][clip]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");

    node->SetClip("nonexistent");
    REQUIRE(node->GetCurrentClipName().empty());
}

TEST_CASE("SetClip without flipbook does nothing", "[flipbook][clip]") {
    FlipbookTestContext tc;
    auto node = NodeFlipbook::Create(tc.context);
    node->SetClip("run");
    REQUIRE(node->GetCurrentClipName().empty());
    REQUIRE(node->GetCurrentFrame() == 0);
}

// ---------------------------------------------------------------------------
// Tests — SetPlaying
// ---------------------------------------------------------------------------

TEST_CASE("SetPlaying without a clip does not start playback", "[flipbook][playing]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetPlaying(true);
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("SetPlaying without a flipbook does not start playback", "[flipbook][playing]") {
    FlipbookTestContext tc;
    auto node = NodeFlipbook::Create(tc.context);
    node->SetPlaying(true);
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("SetPlaying false while already stopped is a no-op", "[flipbook][playing]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    REQUIRE_FALSE(node->IsPlaying());
    node->SetPlaying(false);
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("SetPlaying true after clip is set starts playback", "[flipbook][playing]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);
    REQUIRE(node->IsPlaying());
}

TEST_CASE("SetPlaying false pauses playback", "[flipbook][playing]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);
    node->SetPlaying(false);
    REQUIRE_FALSE(node->IsPlaying());
}

// ---------------------------------------------------------------------------
// Tests — frame advancement
// ---------------------------------------------------------------------------

TEST_CASE("Update with zero durationMs treats frame as 1 ms", "[flipbook][update][duration]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    fb.clips["run"] = MakeClip({ 0, 1, 2, 3 }, 0, IFlipbook::LoopType::Loop);
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    node->Update(1000);

    REQUIRE(node->IsPlaying());
    // Zero-duration frames are clamped to 1 ms; 1000 ms advances through
    // 250 complete loops of the 4-frame clip, landing back at frame 0.
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("Update advances frame at correct duration", "[flipbook][update]") {
    // 84 ms per frame. 83 ms is not enough; 84 ms crosses the threshold.
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook(); // 84 ms per frame
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    REQUIRE(node->GetCurrentFrame() == 0);

    // 83 ms — not quite enough for one step.
    node->Update(83);
    REQUIRE(node->GetCurrentFrame() == 0);

    // 1 more ms pushes accumulated to 84 ms, which meets the 84 ms threshold.
    node->Update(1);
    REQUIRE(node->GetCurrentFrame() == 1);
}

TEST_CASE("Update does not advance frame when paused", "[flipbook][update]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    // Not calling SetPlaying — stays paused.

    node->Update(10000);
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("Update advances multiple frames in one large tick", "[flipbook][update]") {
    // 84 ms per frame. 3 steps = 252 ms; 260 ms crosses 3 boundaries (step 0→1→2→3).
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    node->Update(260);
    REQUIRE(node->GetCurrentFrame() == 3);
}

// ---------------------------------------------------------------------------
// Tests — LoopType::Loop
// ---------------------------------------------------------------------------

TEST_CASE("LoopType::Loop wraps back to first step", "[flipbook][loop]") {
    // "run": 4 steps × 84 ms = 336 ms per loop. 340 ms wraps once to step 0.
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    node->Update(340);
    REQUIRE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("LoopType::Loop continues playing after wrap", "[flipbook][loop]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    node->Update(668); // crosses multiple loop boundaries
    REQUIRE(node->IsPlaying());
}

// ---------------------------------------------------------------------------
// Tests — LoopType::Stop
// ---------------------------------------------------------------------------

TEST_CASE("LoopType::Stop freezes on last step and stops playing", "[flipbook][stop]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    for (int i = 0; i < 4; ++i) {
        fb.frames.push_back({ MakeRect(i * 16, 0, 16, 16), { 8, 8 } });
    }
    fb.clips["hit"] = MakeClip({ 0, 1, 2, 3 }, 84, IFlipbook::LoopType::Stop);
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("hit");
    node->SetPlaying(true);

    node->Update(1000);
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 3); // frozen at last step
}

// ---------------------------------------------------------------------------
// Tests — LoopType::Reset
// ---------------------------------------------------------------------------

TEST_CASE("LoopType::Reset rewinds to first step and stops playing", "[flipbook][reset]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    for (int i = 0; i < 4; ++i) {
        fb.frames.push_back({ MakeRect(i * 16, 0, 16, 16), { 8, 8 } });
    }
    fb.clips["die"] = MakeClip({ 0, 1, 2, 3 }, 100, IFlipbook::LoopType::Reset);
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = NodeFlipbook::Create(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("die");
    node->SetPlaying(true);

    node->Update(1000);
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0); // rewound to first step
}

// ---------------------------------------------------------------------------
// Tests — events
// ---------------------------------------------------------------------------

// Helper: attaches a Group as parent so SendEvent has somewhere to go.
static std::shared_ptr<Group> MakeGroupWithFlipbookChild(
    moth_ui::Context& ctx,
    std::shared_ptr<NodeFlipbook>& outFlipbook) {
    auto group    = Group::Create(ctx);
    auto flipbook = NodeFlipbook::Create(ctx);
    outFlipbook   = flipbook;
    group->AddChild(flipbook);
    return group;
}

TEST_CASE("SetPlaying(true) fires EventFlipbookStarted", "[flipbook][events]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    int startedCount = 0;
    std::string capturedClip;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (auto const* e = event_cast<EventFlipbookStarted>(ev)) {
            ++startedCount;
            capturedClip = std::string(e->GetClipName());
        }
        return true;
    });

    flipbook->Load("dummy.flipbook.json");
    flipbook->SetClip("run");
    flipbook->SetPlaying(true);

    REQUIRE(startedCount == 1);
    REQUIRE(capturedClip == "run");
}

TEST_CASE("SetPlaying(true) twice only fires EventFlipbookStarted once", "[flipbook][events]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    int startedCount = 0;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (ev.GetType() == EventFlipbookStarted::GetStaticType()) { ++startedCount; }
        return true;
    });

    flipbook->Load("dummy.flipbook.json");
    flipbook->SetClip("run");
    flipbook->SetPlaying(true);
    flipbook->SetPlaying(true); // already playing — should not re-fire
    REQUIRE(startedCount == 1);
}

TEST_CASE("LoopType::Stop fires EventFlipbookStopped when clip ends", "[flipbook][events]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    for (int i = 0; i < 4; ++i) {
        fb.frames.push_back({ MakeRect(i * 16, 0, 16, 16), { 8, 8 } });
    }
    fb.clips["hit"] = MakeClip({ 0, 1, 2, 3 }, 84, IFlipbook::LoopType::Stop);
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    int stoppedCount = 0;
    std::string capturedClip;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (auto const* e = event_cast<EventFlipbookStopped>(ev)) {
            ++stoppedCount;
            capturedClip = std::string(e->GetClipName());
        }
        return true;
    });

    flipbook->Load("dummy.flipbook.json");
    flipbook->SetClip("hit");
    flipbook->SetPlaying(true);
    flipbook->Update(1000);

    REQUIRE(stoppedCount == 1);
    REQUIRE(capturedClip == "hit");
}

TEST_CASE("LoopType::Reset fires EventFlipbookStopped when clip ends", "[flipbook][events]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    for (int i = 0; i < 4; ++i) {
        fb.frames.push_back({ MakeRect(i * 16, 0, 16, 16), { 8, 8 } });
    }
    fb.clips["die"] = MakeClip({ 0, 1, 2, 3 }, 100, IFlipbook::LoopType::Reset);
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    int stoppedCount = 0;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (ev.GetType() == EventFlipbookStopped::GetStaticType()) { ++stoppedCount; }
        return true;
    });

    flipbook->Load("dummy.flipbook.json");
    flipbook->SetClip("die");
    flipbook->SetPlaying(true);
    flipbook->Update(1000);

    REQUIRE(stoppedCount == 1);
}

TEST_CASE("LoopType::Loop does not fire EventFlipbookStopped", "[flipbook][events]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook(); // Loop type
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    int stoppedCount = 0;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (ev.GetType() == EventFlipbookStopped::GetStaticType()) { ++stoppedCount; }
        return true;
    });

    flipbook->Load("dummy.flipbook.json");
    flipbook->SetClip("run");
    flipbook->SetPlaying(true);
    flipbook->Update(10000); // many loops

    REQUIRE(stoppedCount == 0);
}

TEST_CASE("EventFlipbookStarted GetNode locks to the firing node", "[flipbook][events][weak_ptr]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    std::shared_ptr<NodeFlipbook> capturedNode;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (auto const* e = event_cast<EventFlipbookStarted>(ev)) {
            capturedNode = e->GetNode().lock();
        }
        return true;
    });

    flipbook->Load("dummy.flipbook.json");
    flipbook->SetClip("run");
    flipbook->SetPlaying(true);

    REQUIRE(capturedNode != nullptr);
    REQUIRE(capturedNode.get() == flipbook.get());
}

TEST_CASE("EventFlipbookStopped GetNode locks to the firing node (Stop)", "[flipbook][events][weak_ptr]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    for (int i = 0; i < 4; ++i) {
        fb.frames.push_back({ MakeRect(i * 16, 0, 16, 16), { 8, 8 } });
    }
    fb.clips["hit"] = MakeClip({ 0, 1, 2, 3 }, 84, IFlipbook::LoopType::Stop);
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    std::shared_ptr<NodeFlipbook> capturedNode;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (auto const* e = event_cast<EventFlipbookStopped>(ev)) {
            capturedNode = e->GetNode().lock();
        }
        return true;
    });

    flipbook->Load("dummy.flipbook.json");
    flipbook->SetClip("hit");
    flipbook->SetPlaying(true);
    flipbook->Update(1000);

    REQUIRE(capturedNode != nullptr);
    REQUIRE(capturedNode.get() == flipbook.get());
}

TEST_CASE("EventFlipbookStopped GetNode locks to the firing node (Reset)", "[flipbook][events][weak_ptr]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    for (int i = 0; i < 4; ++i) {
        fb.frames.push_back({ MakeRect(i * 16, 0, 16, 16), { 8, 8 } });
    }
    fb.clips["die"] = MakeClip({ 0, 1, 2, 3 }, 100, IFlipbook::LoopType::Reset);
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    std::shared_ptr<NodeFlipbook> capturedNode;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (auto const* e = event_cast<EventFlipbookStopped>(ev)) {
            capturedNode = e->GetNode().lock();
        }
        return true;
    });

    flipbook->Load("dummy.flipbook.json");
    flipbook->SetClip("die");
    flipbook->SetPlaying(true);
    flipbook->Update(1000);

    REQUIRE(capturedNode != nullptr);
    REQUIRE(capturedNode.get() == flipbook.get());
}
