#include "moth_ui/layout/layout.h"
#include "moth_ui/layout/layout_entity_clip.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/layout/layout_rect.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/animation/animation_event.h"
#include "moth_ui/graphics/text_alignment.h"
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

using namespace moth_ui;

//NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

// ---- helpers ----------------------------------------------------------------

// RAII wrapper that deletes a temp file on destruction.
struct TempFile {
    std::filesystem::path path;
    explicit TempFile(std::string const& name)
        : path(std::filesystem::temp_directory_path() / name) {}
    ~TempFile() { std::filesystem::remove(path); }
    operator std::filesystem::path const&() const { return path; }
};

// Serialize two layouts with a neutral context and compare the resulting JSON.
static bool jsonEqual(Layout const& a, Layout const& b) {
    LayoutEntity::SerializeContext ctx;
    ctx.m_version = Layout::Version;
    return a.Serialize(ctx) == b.Serialize(ctx);
}

// ---- load result codes ------------------------------------------------------

TEST_CASE("Layout::Load returns DoesNotExist for missing file", "[layout][load]") {
    auto [out, result] = Layout::Load("/tmp/moth_no_such_file.mothui");
    REQUIRE(result == Layout::LoadResult::DoesNotExist);
    REQUIRE(out == nullptr);
}

TEST_CASE("Layout::Load returns IncorrectFormat for malformed JSON", "[layout][load]") {
    TempFile tmp("moth_bad.mothui");
    { std::ofstream f(tmp.path); f << "this is not json {{{"; }

    auto [out, result] = Layout::Load(tmp);
    REQUIRE(result == Layout::LoadResult::IncorrectFormat);
}

TEST_CASE("Layout::Load returns IncorrectFormat for valid JSON without mothui_version", "[layout][load]") {
    TempFile tmp("moth_nover.mothui");
    { std::ofstream f(tmp.path); f << R"({"key":"value"})"; }

    auto [out, result] = Layout::Load(tmp);
    REQUIRE(result == Layout::LoadResult::IncorrectFormat);
}

TEST_CASE("Layout::Load succeeds for valid files", "[layout][load]") {
    TempFile tmp("moth_validate.mothui");
    auto layout = std::make_shared<Layout>();
    REQUIRE(layout->Save(tmp));

    auto result = Layout::Load(tmp).second;
    REQUIRE(result == Layout::LoadResult::Success);
}

TEST_CASE("Layout::Load returns DoesNotExist for missing file", "[layout][load]") {
    auto result = Layout::Load("/tmp/moth_no_such_file.mothui").second;
    REQUIRE(result == Layout::LoadResult::DoesNotExist);
}

// ---- empty layout round-trip ------------------------------------------------

TEST_CASE("Empty layout save/load round-trip", "[layout][serialization]") {
    TempFile tmp("moth_empty.mothui");
    auto original = std::make_shared<Layout>();
    REQUIRE(original->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp); REQUIRE(result == Layout::LoadResult::Success);
    REQUIRE(loaded != nullptr);

    REQUIRE(jsonEqual(*original, *loaded));
}

TEST_CASE("Layout::Load sets the loaded path", "[layout][load]") {
    TempFile tmp("moth_path.mothui");
    auto layout = std::make_shared<Layout>();
    REQUIRE(layout->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp);
    REQUIRE(loaded->GetLoadedPath() == tmp.path);
}

// ---- child entity round-trips -----------------------------------------------

TEST_CASE("Layout with LayoutEntityRect child round-trips", "[layout][serialization]") {
    TempFile tmp("moth_rect.mothui");
    auto original = std::make_shared<Layout>();

    auto rect = std::make_shared<LayoutEntityRect>(original.get());
    rect->m_id = "my_rect";
    rect->m_filled = false;
    original->m_children.push_back(rect);

    REQUIRE(original->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp); REQUIRE(result == Layout::LoadResult::Success);

    REQUIRE(loaded->m_children.size() == 1);
    REQUIRE(loaded->m_children[0]->GetType() == LayoutEntityType::Rect);
    REQUIRE(loaded->m_children[0]->m_id == "my_rect");

    auto* loadedRect = dynamic_cast<LayoutEntityRect*>(loaded->m_children[0].get());
    REQUIRE(loadedRect->m_filled == false);

    REQUIRE(jsonEqual(*original, *loaded));
}

TEST_CASE("Layout with LayoutEntityText child round-trips", "[layout][serialization]") {
    TempFile tmp("moth_text.mothui");
    auto original = std::make_shared<Layout>();

    auto text = std::make_shared<LayoutEntityText>(original.get());
    text->m_id = "label";
    text->m_text = "Hello, world!";
    text->m_fontName = "Arial";
    text->m_fontSize = 24;
    text->m_horizontalAlignment = TextHorizAlignment::Center;
    text->m_verticalAlignment = TextVertAlignment::Middle;
    text->m_dropShadow = true;
    text->m_dropShadowOffset = { 2, 2 };
    original->m_children.push_back(text);

    REQUIRE(original->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp); REQUIRE(result == Layout::LoadResult::Success);

    REQUIRE(loaded->m_children.size() == 1);
    REQUIRE(loaded->m_children[0]->GetType() == LayoutEntityType::Text);

    auto* loadedText = dynamic_cast<LayoutEntityText*>(loaded->m_children[0].get());
    REQUIRE(loadedText->m_text == "Hello, world!");
    REQUIRE(loadedText->m_fontName == "Arial");
    REQUIRE(loadedText->m_fontSize == 24);
    REQUIRE(loadedText->m_horizontalAlignment == TextHorizAlignment::Center);
    REQUIRE(loadedText->m_verticalAlignment == TextVertAlignment::Middle);
    REQUIRE(loadedText->m_dropShadow == true);
    REQUIRE(loadedText->m_dropShadowOffset.x == 2);
    REQUIRE(loadedText->m_dropShadowOffset.y == 2);

    REQUIRE(jsonEqual(*original, *loaded));
}

TEST_CASE("Layout with LayoutEntityClip child round-trips", "[layout][serialization]") {
    TempFile tmp("moth_clip.mothui");
    auto original = std::make_shared<Layout>();

    auto clip = std::make_shared<LayoutEntityClip>(original.get());
    clip->m_id = "scissor_region";
    original->m_children.push_back(clip);

    REQUIRE(original->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp); REQUIRE(result == Layout::LoadResult::Success);

    REQUIRE(loaded->m_children.size() == 1);
    REQUIRE(loaded->m_children[0]->GetType() == LayoutEntityType::Clip);
    REQUIRE(loaded->m_children[0]->m_id == "scissor_region");

    REQUIRE(jsonEqual(*original, *loaded));
}

TEST_CASE("Layout preserves child ordering across round-trip", "[layout][serialization]") {
    TempFile tmp("moth_order.mothui");
    auto original = std::make_shared<Layout>();

    for (int i = 0; i < 5; ++i) {
        auto rect = std::make_shared<LayoutEntityRect>(original.get());
        rect->m_id = "rect_" + std::to_string(i);
        original->m_children.push_back(rect);
    }

    REQUIRE(original->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp); REQUIRE(result == Layout::LoadResult::Success);

    REQUIRE(loaded->m_children.size() == 5);
    for (int i = 0; i < 5; ++i) {
        REQUIRE(loaded->m_children[i]->m_id == "rect_" + std::to_string(i));
    }
}

// ---- animation clips --------------------------------------------------------

TEST_CASE("Layout animation clips round-trip", "[layout][serialization][animation]") {
    TempFile tmp("moth_clips.mothui");
    auto original = std::make_shared<Layout>();

    auto clip = std::make_unique<AnimationClip>();
    clip->m_name = "walk";
    clip->m_startFrame = 0;
    clip->m_endFrame = 24;
    clip->m_fps = 24.0f;
    clip->m_loopType = AnimationClip::LoopType::Loop;
    original->m_clips.push_back(std::move(clip));

    auto clip2 = std::make_unique<AnimationClip>();
    clip2->m_name = "idle";
    clip2->m_startFrame = 30;
    clip2->m_endFrame = 60;
    original->m_clips.push_back(std::move(clip2));

    REQUIRE(original->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp); REQUIRE(result == Layout::LoadResult::Success);

    REQUIRE(loaded->m_clips.size() == 2);
    REQUIRE(loaded->m_clips[0]->m_name == "walk");
    REQUIRE(loaded->m_clips[0]->m_startFrame == 0);
    REQUIRE(loaded->m_clips[0]->m_endFrame == 24);
    REQUIRE(loaded->m_clips[0]->m_fps == Catch::Approx(24.0f));
    REQUIRE(loaded->m_clips[0]->m_loopType == AnimationClip::LoopType::Loop);
    REQUIRE(loaded->m_clips[1]->m_name == "idle");

    REQUIRE(jsonEqual(*original, *loaded));
}

// ---- animation events -------------------------------------------------------

TEST_CASE("Layout animation events round-trip", "[layout][serialization][animation]") {
    TempFile tmp("moth_events.mothui");
    auto original = std::make_shared<Layout>();

    original->m_events.push_back(std::make_unique<AnimationEvent>(5,  "footstep"));
    original->m_events.push_back(std::make_unique<AnimationEvent>(12, "spawn"));

    REQUIRE(original->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp); REQUIRE(result == Layout::LoadResult::Success);

    REQUIRE(loaded->m_events.size() == 2);
    REQUIRE(loaded->m_events[0]->m_frame == 5);
    REQUIRE(loaded->m_events[0]->m_name == "footstep");
    REQUIRE(loaded->m_events[1]->m_frame == 12);
    REQUIRE(loaded->m_events[1]->m_name == "spawn");

    REQUIRE(jsonEqual(*original, *loaded));
}

// ---- keyframe tracks --------------------------------------------------------

TEST_CASE("Layout entity keyframe tracks round-trip", "[layout][serialization][animation]") {
    TempFile tmp("moth_tracks.mothui");
    auto original = std::make_shared<Layout>();

    auto rect = std::make_shared<LayoutEntityRect>(original.get());
    rect->m_id = "animated_rect";

    // Set non-default bounds at frame 0 and frame 10
    LayoutRect bounds0 = MakeDefaultLayoutRect();
    bounds0.offset.topLeft = { 10.0f, 20.0f };
    rect->SetBounds(bounds0, 0);

    LayoutRect bounds10 = MakeDefaultLayoutRect();
    bounds10.offset.topLeft = { 100.0f, 200.0f };
    rect->SetBounds(bounds10, 10);

    original->m_children.push_back(rect);

    REQUIRE(original->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp); REQUIRE(result == Layout::LoadResult::Success);

    REQUIRE(loaded->m_children.size() == 1);
    auto* loadedRect = dynamic_cast<LayoutEntityRect*>(loaded->m_children[0].get());

    auto b0 = loadedRect->GetBoundsAtFrame(0.0f);
    REQUIRE(b0.offset.topLeft.x == Catch::Approx(10.0f));
    REQUIRE(b0.offset.topLeft.y == Catch::Approx(20.0f));

    auto b10 = loadedRect->GetBoundsAtFrame(10.0f);
    REQUIRE(b10.offset.topLeft.x == Catch::Approx(100.0f));
    REQUIRE(b10.offset.topLeft.y == Catch::Approx(200.0f));

    REQUIRE(jsonEqual(*original, *loaded));
}

// ---- extra data -------------------------------------------------------------

TEST_CASE("Layout extra data round-trips", "[layout][serialization]") {
    TempFile tmp("moth_extra.mothui");
    auto original = std::make_shared<Layout>();
    original->GetExtraData()["editor_zoom"] = 1.5;
    original->GetExtraData()["last_frame"] = 42;

    REQUIRE(original->Save(tmp));

    auto [loaded, result] = Layout::Load(tmp); REQUIRE(result == Layout::LoadResult::Success);

    REQUIRE(loaded->GetExtraData()["editor_zoom"].get<double>() == Catch::Approx(1.5));
    REQUIRE(loaded->GetExtraData()["last_frame"].get<int>() == 42);
}

// ---- Layout::Version --------------------------------------------------------

TEST_CASE("Saved layout contains current version number", "[layout][serialization]") {
    TempFile tmp("moth_version.mothui");
    auto layout = std::make_shared<Layout>();
    REQUIRE(layout->Save(tmp));

    std::ifstream f(tmp.path);
    auto j = nlohmann::json::parse(f);
    REQUIRE(j["mothui_version"].get<int>() == Layout::Version);
}

//NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

