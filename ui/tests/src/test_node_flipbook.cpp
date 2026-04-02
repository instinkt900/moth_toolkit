#include "mock_context.h"
#include "moth_ui/events/event_flipbook.h"
#include "moth_ui/graphics/iflipbook.h"
#include "moth_ui/graphics/iimage.h"
#include "moth_ui/iflipbook_factory.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/nodes/node_flipbook.h"
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
    void ImGui(IntVec2 const&, FloatVec2 const&, FloatVec2 const&) const override {}
};

class MockFlipbook : public IFlipbook {
public:
    mutable MockImage image;
    SheetDesc sheetDesc;
    std::map<std::string, ClipDesc, std::less<>> clips;

    IImage& GetImage() const override { return image; }
    void GetSheetDesc(SheetDesc& outDesc) const override { outDesc = sheetDesc; }

    std::string_view GetClipName(int index) const override {
        int i = 0;
        for (auto const& [name, _] : clips) {
            if (i++ == index) return name;
        }
        return {};
    }

    bool GetClipDesc(std::string_view name, ClipDesc& outDesc) const override {
        auto it = clips.find(name);
        if (it == clips.end()) return false;
        outDesc = it->second;
        return true;
    }
};

class MockFlipbookFactory : public IFlipbookFactory {
public:
    MockFlipbook* nextFlipbook = nullptr; // non-owning pointer to the flipbook to return
    int getFlipbookCalls = 0;
    std::filesystem::path lastRequestedPath;

    void FlushCache() override {}

    std::unique_ptr<IFlipbook> GetFlipbook(std::filesystem::path const& path) override {
        ++getFlipbookCalls;
        lastRequestedPath = path;
        if (nextFlipbook == nullptr) return nullptr;
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

// Build a MockFlipbook with a 4×1 sheet and one clip named "run" (frames 0–3, 12 FPS, Loop).
static MockFlipbook MakeSimpleFlipbook() {
    MockFlipbook fb;
    fb.sheetDesc.FrameDimensions = { 16, 16 };
    fb.sheetDesc.SheetCells      = { 4, 1 };
    fb.sheetDesc.MaxFrames       = 4;
    fb.sheetDesc.NumClips        = 1;
    fb.clips["run"] = IFlipbook::ClipDesc{ 0, 3, 12, IFlipbook::LoopType::Loop };
    return fb;
}

// ---------------------------------------------------------------------------
// Tests — Load path forwarding
// ---------------------------------------------------------------------------

TEST_CASE("Load with null factory clears all flipbook state", "[flipbook][load]") {
    // Context has no flipbook factory — Load should leave the node fully cleared.
    MockContext mc;
    auto node = std::make_shared<NodeFlipbook>(mc.context);

    // Pre-load a valid flipbook so there is state to clear.
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto primed = std::make_shared<NodeFlipbook>(tc.context);
    primed->SetInitialClipName("run");
    primed->Load("dummy.flipbook.json");
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
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->SetInitialClipName("run");
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

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->SetInitialClipName("run");
    node->Load("dummy.flipbook.json");
    REQUIRE(node->GetFlipbook() != nullptr);

    // Now make the factory return null and reload.
    tc.flipbookFactory.nextFlipbook = nullptr;
    node->Load("dummy.flipbook.json");

    REQUIRE(node->GetFlipbook() == nullptr);
    REQUIRE(node->GetCurrentClipName().empty());
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("Load with malformed SheetDesc (zero columns) clears flipbook", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    fb.sheetDesc.SheetCells = { 0, 1 }; // invalid — zero columns
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");

    REQUIRE(node->GetFlipbook() == nullptr);
    REQUIRE(node->GetCurrentClipName().empty());
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("Load with malformed SheetDesc (zero rows) clears flipbook", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    fb.sheetDesc.SheetCells = { 4, 0 }; // invalid — zero rows
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");

    REQUIRE(node->GetFlipbook() == nullptr);
}

TEST_CASE("Load with malformed SheetDesc (zero frame dimensions) clears flipbook", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    fb.sheetDesc.FrameDimensions = { 0, 16 }; // invalid — zero width
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");

    REQUIRE(node->GetFlipbook() == nullptr);
}

TEST_CASE("Load forwards path to flipbook factory", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load(std::filesystem::path{ "dummy.flipbook.json" });

    REQUIRE(tc.flipbookFactory.getFlipbookCalls == 1);
    REQUIRE(tc.flipbookFactory.lastRequestedPath == std::filesystem::path{ "dummy.flipbook.json" });
}

TEST_CASE("Load resets playback state before loading", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->SetInitialClipName("run");
    node->Load("dummy.flipbook.json");
    node->SetPlaying(true);
    node->Update(84); // advance one frame

    REQUIRE(node->GetCurrentFrame() == 1);

    // Reload — frame, time, and playing should all reset.
    node->Load("dummy.flipbook.json");
    REQUIRE(node->GetCurrentFrame() == 0);
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("Load with initial clip name activates that clip", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->SetInitialClipName("run");
    node->Load("dummy.flipbook.json");

    REQUIRE(node->GetCurrentClipName() == "run");
    REQUIRE(node->GetCurrentFrame() == 0); // Start of clip
}

TEST_CASE("Load with no initial clip name leaves clip unset", "[flipbook][load]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    // m_initialClipName defaults to empty
    node->Load("dummy.flipbook.json");

    REQUIRE(node->GetCurrentClipName().empty());
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("Autoplay true starts playback after Load", "[flipbook][load][autoplay]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->SetInitialClipName("run");
    node->SetAutoplay(true);
    node->Load("dummy.flipbook.json");

    REQUIRE(node->IsPlaying());
}

TEST_CASE("Autoplay false does not start playback after Load", "[flipbook][load][autoplay]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->SetInitialClipName("run");
    node->SetAutoplay(false);
    node->Load("dummy.flipbook.json");

    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("Autoplay true with no initial clip does not start playback", "[flipbook][load][autoplay]") {
    // Autoplay requires a clip to be set — no clip means SetPlaying is a no-op.
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->SetAutoplay(true);
    // No SetInitialClipName — m_initialClipName is empty.
    node->Load("dummy.flipbook.json");

    REQUIRE_FALSE(node->IsPlaying());
}

// ---------------------------------------------------------------------------
// Tests — no flipbook loaded
// ---------------------------------------------------------------------------

TEST_CASE("NodeFlipbook default state: no clip, not playing, frame 0", "[flipbook]") {
    FlipbookTestContext tc;
    // Factory returns nullptr — no flipbook loaded.
    auto node = std::make_shared<NodeFlipbook>(tc.context);

    REQUIRE(node->GetFlipbook() == nullptr);
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
    REQUIRE(node->GetCurrentClipName().empty());
}

TEST_CASE("NodeFlipbook Update without flipbook does not crash", "[flipbook]") {
    FlipbookTestContext tc;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Update(1000);
    REQUIRE(node->GetCurrentFrame() == 0);
}

// ---------------------------------------------------------------------------
// Tests — SetClip
// ---------------------------------------------------------------------------

TEST_CASE("SetClip with valid name sets frame to clip start", "[flipbook][clip]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    fb.sheetDesc.FrameDimensions = { 16, 16 };
    fb.sheetDesc.SheetCells      = { 6, 1 };
    fb.sheetDesc.MaxFrames       = 6;
    fb.sheetDesc.NumClips        = 2;
    fb.clips["idle"] = IFlipbook::ClipDesc{ 0, 2, 10, IFlipbook::LoopType::Loop };
    fb.clips["run"]  = IFlipbook::ClipDesc{ 3, 5, 12, IFlipbook::LoopType::Loop };

    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");

    node->SetClip("run");
    REQUIRE(node->GetCurrentClipName() == "run");
    REQUIRE(node->GetCurrentFrame() == 3);
}

TEST_CASE("SetClip with invalid name clears clip", "[flipbook][clip]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");

    node->SetClip("nonexistent");
    REQUIRE(node->GetCurrentClipName().empty());
}

TEST_CASE("SetClip without flipbook does nothing", "[flipbook][clip]") {
    FlipbookTestContext tc;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
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
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    // No clip set — SetPlaying should be a no-op.
    node->SetPlaying(true);
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("SetPlaying without a flipbook does not start playback", "[flipbook][playing]") {
    FlipbookTestContext tc;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->SetPlaying(true);
    REQUIRE_FALSE(node->IsPlaying());
}

TEST_CASE("SetPlaying false while already stopped is a no-op", "[flipbook][playing]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
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
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);
    REQUIRE(node->IsPlaying());
}

TEST_CASE("SetPlaying false pauses playback", "[flipbook][playing]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);
    node->SetPlaying(false);
    REQUIRE_FALSE(node->IsPlaying());
}

// ---------------------------------------------------------------------------
// Tests — frame advancement
// ---------------------------------------------------------------------------

TEST_CASE("Update with FPS = 0 does not crash and frame stays at start", "[flipbook][update][fps]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    fb.clips["run"].FPS = 0;
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    node->Update(1000);

    REQUIRE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("Update with negative FPS does not crash and frame stays at start", "[flipbook][update][fps]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    fb.clips["run"].FPS = -12;
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    node->Update(1000);

    REQUIRE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("Update advances frame at correct FPS", "[flipbook][update]") {
    // Clip is 12 FPS → one frame every ~83.3 ms.
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook(); // "run": frames 0–3, 12 FPS, Loop
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    REQUIRE(node->GetCurrentFrame() == 0);

    // 83 ms — not quite enough for one frame.
    node->Update(83);
    REQUIRE(node->GetCurrentFrame() == 0);

    // 1 more ms pushes accumulated to 84 ms, which crosses 83.33 ms threshold.
    node->Update(1);
    REQUIRE(node->GetCurrentFrame() == 1);
}

TEST_CASE("Update does not advance frame when paused", "[flipbook][update]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    // Not calling SetPlaying — stays paused.

    node->Update(10000);
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("Update advances multiple frames in one large tick", "[flipbook][update]") {
    // 12 FPS → 83.33 ms/frame. 3 frames = ~250 ms.
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    // Advance 260 ms — well past 3 frame boundaries at 83.33 ms/frame (0 → 1 → 2 → 3).
    node->Update(260);
    REQUIRE(node->GetCurrentFrame() == 3);
}

// ---------------------------------------------------------------------------
// Tests — LoopType::Loop
// ---------------------------------------------------------------------------

TEST_CASE("LoopType::Loop wraps frame back to Start", "[flipbook][loop]") {
    // "run": frames 0–3, 12 FPS, Loop. After frame 3, wraps to 0.
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    // Advance past end: 4 frames worth (frame 0→1→2→3→0).
    node->Update(340); // 4 * 83.33 ms ≈ 333 ms, use 340 to be safe
    REQUIRE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 0);
}

TEST_CASE("LoopType::Loop continues playing after wrap", "[flipbook][loop]") {
    FlipbookTestContext tc;
    MockFlipbook fb = MakeSimpleFlipbook();
    tc.flipbookFactory.nextFlipbook = &fb;
    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("run");
    node->SetPlaying(true);

    // Advance two full loops (8 frames).
    node->Update(668); // 8 * 83.33 ms ≈ 666 ms
    REQUIRE(node->IsPlaying());
}

// ---------------------------------------------------------------------------
// Tests — LoopType::Stop
// ---------------------------------------------------------------------------

TEST_CASE("LoopType::Stop freezes on End frame and stops playing", "[flipbook][stop]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    fb.sheetDesc.FrameDimensions = { 16, 16 };
    fb.sheetDesc.SheetCells      = { 4, 1 };
    fb.sheetDesc.MaxFrames       = 4;
    fb.sheetDesc.NumClips        = 1;
    fb.clips["hit"] = IFlipbook::ClipDesc{ 0, 3, 12, IFlipbook::LoopType::Stop };
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("hit");
    node->SetPlaying(true);

    // Advance well past the end.
    node->Update(1000);
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 3); // frozen at End
}

// ---------------------------------------------------------------------------
// Tests — LoopType::Reset
// ---------------------------------------------------------------------------

TEST_CASE("LoopType::Reset rewinds to Start and stops playing", "[flipbook][reset]") {
    FlipbookTestContext tc;
    MockFlipbook fb;
    fb.sheetDesc.FrameDimensions = { 16, 16 };
    fb.sheetDesc.SheetCells      = { 6, 1 };
    fb.sheetDesc.MaxFrames       = 6;
    fb.sheetDesc.NumClips        = 1;
    fb.clips["die"] = IFlipbook::ClipDesc{ 2, 5, 10, IFlipbook::LoopType::Reset };
    tc.flipbookFactory.nextFlipbook = &fb;

    auto node = std::make_shared<NodeFlipbook>(tc.context);
    node->Load("dummy.flipbook.json");
    node->SetClip("die");
    node->SetPlaying(true);

    // Advance past the end.
    node->Update(1000);
    REQUIRE_FALSE(node->IsPlaying());
    REQUIRE(node->GetCurrentFrame() == 2); // rewound to Start
}

// ---------------------------------------------------------------------------
// Tests — events
// ---------------------------------------------------------------------------

// Helper: attaches a Group as parent so SendEventUp has somewhere to go.
static std::shared_ptr<Group> MakeGroupWithFlipbookChild(
    moth_ui::Context& ctx,
    std::shared_ptr<NodeFlipbook>& outFlipbook) {
    auto group    = std::make_shared<Group>(ctx);
    auto flipbook = std::make_shared<NodeFlipbook>(ctx);
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
        if (ev.GetType() == EventFlipbookStarted::GetStaticType()) {
            ++startedCount;
            capturedClip = std::string(static_cast<EventFlipbookStarted const&>(ev).GetClipName());
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
        if (ev.GetType() == EventFlipbookStarted::GetStaticType()) ++startedCount;
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
    fb.sheetDesc.FrameDimensions = { 16, 16 };
    fb.sheetDesc.SheetCells      = { 4, 1 };
    fb.sheetDesc.MaxFrames       = 4;
    fb.sheetDesc.NumClips        = 1;
    fb.clips["hit"] = IFlipbook::ClipDesc{ 0, 3, 12, IFlipbook::LoopType::Stop };
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    int stoppedCount = 0;
    std::string capturedClip;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (ev.GetType() == EventFlipbookStopped::GetStaticType()) {
            ++stoppedCount;
            capturedClip = std::string(static_cast<EventFlipbookStopped const&>(ev).GetClipName());
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
    fb.sheetDesc.FrameDimensions = { 16, 16 };
    fb.sheetDesc.SheetCells      = { 4, 1 };
    fb.sheetDesc.MaxFrames       = 4;
    fb.sheetDesc.NumClips        = 1;
    fb.clips["die"] = IFlipbook::ClipDesc{ 0, 3, 10, IFlipbook::LoopType::Reset };
    tc.flipbookFactory.nextFlipbook = &fb;

    std::shared_ptr<NodeFlipbook> flipbook;
    auto group = MakeGroupWithFlipbookChild(tc.context, flipbook);

    int stoppedCount = 0;
    group->SetEventHandler([&](Node*, Event const& ev) -> bool {
        if (ev.GetType() == EventFlipbookStopped::GetStaticType()) ++stoppedCount;
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
        if (ev.GetType() == EventFlipbookStopped::GetStaticType()) ++stoppedCount;
        return true;
    });

    flipbook->Load("dummy.flipbook.json");
    flipbook->SetClip("run");
    flipbook->SetPlaying(true);
    flipbook->Update(10000); // many loops

    REQUIRE(stoppedCount == 0);
}
