#include "mock_context.h"
#include "moth_ui/nodes/node.h"
#include "moth_ui/context.h"
#include "moth_ui/events/event_mouse.h"
#include "moth_ui/utils/color.h"
#include "moth_ui/utils/rect.h"
#include "moth_ui/graphics/blend_mode.h"
#include "moth_ui/layout/layout_rect.h"
#include <catch2/catch_all.hpp>
#include <memory>
#include <stdexcept>

using namespace moth_ui;

// Node must be created via Node::Create to satisfy enable_shared_from_this.

TEST_CASE("Context throws on null arguments", "[context][construction]") {
    MockRenderer renderer;
    MockImageFactory imageFactory;
    MockFontFactory fontFactory;
    REQUIRE_THROWS_AS(Context(nullptr, &fontFactory, &renderer), std::invalid_argument);
    REQUIRE_THROWS_AS(Context(&imageFactory, nullptr, &renderer), std::invalid_argument);
    REQUIRE_THROWS_AS(Context(&imageFactory, &fontFactory, nullptr), std::invalid_argument);
}

TEST_CASE("Node id get and set", "[node][properties]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    REQUIRE(node->GetId().empty());
    node->SetId("my_node");
    REQUIRE(node->GetId() == "my_node");
}

TEST_CASE("Node visibility get and set", "[node][properties]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    REQUIRE(node->IsVisible());
    node->SetVisible(false);
    REQUIRE_FALSE(node->IsVisible());
    node->SetVisible(true);
    REQUIRE(node->IsVisible());
}

TEST_CASE("Node show rect get and set", "[node][properties]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    REQUIRE_FALSE(node->GetShowRect());
    node->SetShowRect(true);
    REQUIRE(node->GetShowRect());
}

TEST_CASE("Node default color is white", "[node][properties]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    auto const& color = node->GetColor();
    REQUIRE(color.r == Catch::Approx(1.0f));
    REQUIRE(color.g == Catch::Approx(1.0f));
    REQUIRE(color.b == Catch::Approx(1.0f));
    REQUIRE(color.a == Catch::Approx(1.0f));
}

TEST_CASE("Node color get and set", "[node][properties]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    Color red{ 1.0f, 0.0f, 0.0f, 1.0f };
    node->SetColor(red);
    REQUIRE(node->GetColor().r == Catch::Approx(1.0f));
    REQUIRE(node->GetColor().g == Catch::Approx(0.0f));
    REQUIRE(node->GetColor().b == Catch::Approx(0.0f));
}

TEST_CASE("Node default blend mode is Replace", "[node][properties]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    REQUIRE(node->GetBlendMode() == BlendMode::Replace);
}

TEST_CASE("Node blend mode get and set", "[node][properties]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    node->SetBlendMode(BlendMode::Alpha);
    REQUIRE(node->GetBlendMode() == BlendMode::Alpha);
}

TEST_CASE("Node default parent is null", "[node][hierarchy]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    REQUIRE(node->GetParent() == nullptr);
}

TEST_CASE("Node layout entity is null without layout", "[node][layout]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    REQUIRE(node->GetLayoutEntity() == nullptr);
}

TEST_CASE("Node GetLayoutRect is accessible", "[node][layout]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    auto& lr = node->GetLayoutRect();
    // LayoutRect members are accessible — just verify no crash
    (void)lr;
    SUCCEED();
}

TEST_CASE("Node SetScreenRect affects IsInBounds", "[node][bounds]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    node->SetScreenRect(MakeRect(10, 10, 100, 100));

    REQUIRE(node->IsInBounds(IntVec2{ 50, 50 }));
    REQUIRE_FALSE(node->IsInBounds(IntVec2{ 5, 5 }));
    REQUIRE_FALSE(node->IsInBounds(IntVec2{ 200, 200 }));
}

TEST_CASE("Node IsInBounds on boundary", "[node][bounds]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    node->SetScreenRect(MakeRect(0, 0, 100, 100));

    REQUIRE(node->IsInBounds(IntVec2{ 0, 0 }));
    REQUIRE(node->IsInBounds(IntVec2{ 100, 100 }));
}

TEST_CASE("Node event handler is called on event", "[node][events]") {
    MockContext mc;
    auto node = Node::Create(mc.context);

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
    auto node = Node::Create(mc.context);
    REQUIRE(node->FindChild("anything") == nullptr);
}

TEST_CASE("Node default rotation is zero", "[node][rotation]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    REQUIRE(node->GetRotation() == Catch::Approx(0.0f));
}

TEST_CASE("Node rotation get and set", "[node][rotation]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    node->SetScreenRect(MakeRect(0, 0, 100, 100));
    node->SetRotation(45.0f);
    REQUIRE(node->GetRotation() == Catch::Approx(45.0f));
}

TEST_CASE("Node IsInBounds unrotated accepts interior point", "[node][rotation][bounds]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    node->SetScreenRect(MakeRect(0, 0, 100, 100));
    REQUIRE(node->IsInBounds(IntVec2{ 50, 50 }));
}

TEST_CASE("Node IsInBounds unrotated rejects exterior point", "[node][rotation][bounds]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    node->SetScreenRect(MakeRect(0, 0, 100, 100));
    REQUIRE_FALSE(node->IsInBounds(IntVec2{ 150, 50 }));
}

TEST_CASE("Node IsInBounds with 90-degree rotation", "[node][rotation][bounds]") {
    // 100x100 node at (0,0), rotated 90 degrees CW around its centre (50,50).
    // The bounding region does not change for a square; (50,50) must remain inside.
    MockContext mc;
    auto node = Node::Create(mc.context);
    node->SetScreenRect(MakeRect(0, 0, 100, 100));
    node->SetRotation(90.0f);
    REQUIRE(node->IsInBounds(IntVec2{ 50, 50 }));
}

TEST_CASE("Node IsInBounds with 45-degree rotation rejects corner that moved out", "[node][rotation][bounds]") {
    // 100x100 node at (0,0) rotated 45 CW around centre (50,50).
    // The original top-right corner (100,0) in world space moves to roughly (50+71, 50) = (121, 50).
    // The world point (95, 5) — near the original top-right — is outside the rotated rect.
    MockContext mc;
    auto node = Node::Create(mc.context);
    node->SetScreenRect(MakeRect(0, 0, 100, 100));
    node->SetRotation(45.0f);
    REQUIRE_FALSE(node->IsInBounds(IntVec2{ 95, 5 }));
}

TEST_CASE("Node TranslatePosition maps world point to local space", "[node][rotation][bounds]") {
    MockContext mc;
    auto node = Node::Create(mc.context);
    // No rotation: world point (30,40) on a node at (10,10) -> local (20,30)
    node->SetScreenRect(MakeRect(10, 10, 110, 110));
    auto const local = node->TranslatePosition(IntVec2{ 30, 40 });
    REQUIRE(local.x == Catch::Approx(20.0f).margin(1.0f));
    REQUIRE(local.y == Catch::Approx(30.0f).margin(1.0f));
}
