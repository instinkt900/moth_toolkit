#include "mock_context.h"
#include "moth_ui/nodes/node.h"
#include "moth_ui/events/event_mouse.h"
#include "moth_ui/utils/color.h"
#include "moth_ui/graphics/blend_mode.h"
#include "moth_ui/layout/layout_rect.h"
#include <catch2/catch_all.hpp>
#include <memory>

using namespace moth_ui;

// Node is not abstract — use make_shared to satisfy enable_shared_from_this.

TEST_CASE("Node id get and set", "[node][properties]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    REQUIRE(node->GetId().empty());
    node->SetId("my_node");
    REQUIRE(node->GetId() == "my_node");
}

TEST_CASE("Node visibility get and set", "[node][properties]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    REQUIRE(node->IsVisible());
    node->SetVisible(false);
    REQUIRE_FALSE(node->IsVisible());
    node->SetVisible(true);
    REQUIRE(node->IsVisible());
}

TEST_CASE("Node show rect get and set", "[node][properties]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    REQUIRE_FALSE(node->GetShowRect());
    node->SetShowRect(true);
    REQUIRE(node->GetShowRect());
}

TEST_CASE("Node default color is white", "[node][properties]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    auto const& color = node->GetColor();
    REQUIRE(color.r == Catch::Approx(1.0f));
    REQUIRE(color.g == Catch::Approx(1.0f));
    REQUIRE(color.b == Catch::Approx(1.0f));
    REQUIRE(color.a == Catch::Approx(1.0f));
}

TEST_CASE("Node color get and set", "[node][properties]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    Color red{ 1.0f, 0.0f, 0.0f, 1.0f };
    node->SetColor(red);
    REQUIRE(node->GetColor().r == Catch::Approx(1.0f));
    REQUIRE(node->GetColor().g == Catch::Approx(0.0f));
    REQUIRE(node->GetColor().b == Catch::Approx(0.0f));
}

TEST_CASE("Node default blend mode is Replace", "[node][properties]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    REQUIRE(node->GetBlendMode() == BlendMode::Replace);
}

TEST_CASE("Node blend mode get and set", "[node][properties]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    node->SetBlendMode(BlendMode::Alpha);
    REQUIRE(node->GetBlendMode() == BlendMode::Alpha);
}

TEST_CASE("Node default parent is null", "[node][hierarchy]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    REQUIRE(node->GetParent() == nullptr);
}

TEST_CASE("Node layout entity is null without layout", "[node][layout]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    REQUIRE(node->GetLayoutEntity() == nullptr);
}

TEST_CASE("Node GetLayoutRect is accessible", "[node][layout]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    auto& lr = node->GetLayoutRect();
    // LayoutRect members are accessible — just verify no crash
    (void)lr;
    SUCCEED();
}

TEST_CASE("Node SetScreenRect affects IsInBounds", "[node][bounds]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    node->SetScreenRect(MakeRect(10, 10, 100, 100));

    REQUIRE(node->IsInBounds(IntVec2{ 50, 50 }));
    REQUIRE_FALSE(node->IsInBounds(IntVec2{ 5, 5 }));
    REQUIRE_FALSE(node->IsInBounds(IntVec2{ 200, 200 }));
}

TEST_CASE("Node IsInBounds on boundary", "[node][bounds]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    node->SetScreenRect(MakeRect(0, 0, 100, 100));

    REQUIRE(node->IsInBounds(IntVec2{ 0, 0 }));
    REQUIRE(node->IsInBounds(IntVec2{ 100, 100 }));
}

TEST_CASE("Node event handler is called on event", "[node][events]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);

    int callCount = 0;
    node->SetEventHandler([&](Node*, Event const&) {
        ++callCount;
        return true;
    });

    EventMouseDown ev(MouseButton::Left, IntVec2{ 0, 0 });
    node->OnEvent(ev);
    REQUIRE(callCount == 1);
}

TEST_CASE("Node FindChild returns nullptr for leaf node", "[node][hierarchy]") {
    MockContext mc;
    auto node = std::make_shared<Node>(mc.context);
    REQUIRE(node->FindChild("anything") == nullptr);
}
