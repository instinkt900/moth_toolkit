#include "mock_context.h"
#include "moth_ui/nodes/node_clip.h"
#include "moth_ui/nodes/node_rect.h"
#include "moth_ui/nodes/node_text.h"
#include "moth_ui/nodes/node_image.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/layout/layout_entity_clip.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/graphics/text_alignment.h"
#include "moth_ui/graphics/image_scale_type.h"
#include <catch2/catch_all.hpp>
#include <memory>

using namespace moth_ui;

// ---- AnimationController always valid ---------------------------------------

TEST_CASE("NodeRect default constructor provides a valid AnimationController", "[node][animation][rect]") {
    MockContext mc;
    auto node = std::make_shared<NodeRect>(mc.context);
    REQUIRE_NOTHROW(node->GetAnimationController());
}

TEST_CASE("NodeText default constructor provides a valid AnimationController", "[node][animation][text]") {
    MockContext mc;
    auto node = std::make_shared<NodeText>(mc.context);
    REQUIRE_NOTHROW(node->GetAnimationController());
}

TEST_CASE("NodeImage default constructor provides a valid AnimationController", "[node][animation][image]") {
    MockContext mc;
    auto node = std::make_shared<NodeImage>(mc.context);
    REQUIRE_NOTHROW(node->GetAnimationController());
}

TEST_CASE("NodeClip default constructor provides a valid AnimationController", "[node][animation][clip]") {
    MockContext mc;
    auto node = std::make_shared<NodeClip>(mc.context);
    REQUIRE_NOTHROW(node->GetAnimationController());
}

TEST_CASE("NodeClip loads layout rect from entity", "[node][entity][clip]") {
    LayoutRect layoutRect;
    layoutRect.offset.bottomRight = { 100.0f, 100.0f };
    auto entity = std::make_shared<LayoutEntityClip>(layoutRect);

    MockContext mc;
    auto node = std::make_shared<NodeClip>(mc.context, entity);

    auto const& lr = node->GetLayoutRect();
    REQUIRE(lr.offset.bottomRight.x == 100.0f);
    REQUIRE(lr.offset.bottomRight.y == 100.0f);
}

TEST_CASE("NodeClip set screen rect overrides layout", "[node][entity][clip]") {
    auto entity = std::make_shared<LayoutEntityClip>(LayoutRect{});
    MockContext mc;
    auto node = std::make_shared<NodeClip>(mc.context, entity);

    IntRect newRect{ { 10, 20 }, { 60, 80 } };
    node->SetScreenRect(newRect);

    auto const& sr = node->GetScreenRect();
    REQUIRE(sr.topLeft.x == 10);
    REQUIRE(sr.topLeft.y == 20);
    REQUIRE(sr.bottomRight.x == 60);
    REQUIRE(sr.bottomRight.y == 80);
}

// ---- NodeRect ---------------------------------------------------------------

TEST_CASE("NodeRect reads m_filled from LayoutEntityRect on construction", "[node][entity][rect]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityRect>(nullptr);
    entity->m_filled = false;

    auto node = std::make_shared<NodeRect>(mc.context, entity);
    REQUIRE(node->IsFilled() == false);
}

TEST_CASE("NodeRect m_filled defaults to true when entity has default value", "[node][entity][rect]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityRect>(nullptr);
    // m_filled defaults to true in LayoutEntityRect

    auto node = std::make_shared<NodeRect>(mc.context, entity);
    REQUIRE(node->IsFilled() == true);
}

// ---- NodeText ---------------------------------------------------------------

TEST_CASE("NodeText reads text from LayoutEntityText on construction", "[node][entity][text]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityText>(nullptr);
    entity->m_text = "hello";

    auto node = std::make_shared<NodeText>(mc.context, entity);
    REQUIRE(node->GetText() == "hello");
}

TEST_CASE("NodeText reads horizontal alignment from LayoutEntityText on construction", "[node][entity][text]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityText>(nullptr);
    entity->m_horizontalAlignment = TextHorizAlignment::Center;

    auto node = std::make_shared<NodeText>(mc.context, entity);
    REQUIRE(node->GetHorizontalAlignment() == TextHorizAlignment::Center);
}

TEST_CASE("NodeText reads vertical alignment from LayoutEntityText on construction", "[node][entity][text]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityText>(nullptr);
    entity->m_verticalAlignment = TextVertAlignment::Middle;

    auto node = std::make_shared<NodeText>(mc.context, entity);
    REQUIRE(node->GetVerticalAlignment() == TextVertAlignment::Middle);
}

TEST_CASE("NodeText reads drop shadow from LayoutEntityText on construction", "[node][entity][text]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityText>(nullptr);
    entity->m_dropShadow = true;
    entity->m_dropShadowOffset = { 3, 4 };

    auto node = std::make_shared<NodeText>(mc.context, entity);
    REQUIRE(node->IsDropShadow() == true);
    REQUIRE(node->GetDropShadowOffset().x == 3);
    REQUIRE(node->GetDropShadowOffset().y == 4);
}

// ---- NodeImage --------------------------------------------------------------

TEST_CASE("NodeImage reads image scale type from LayoutEntityImage on construction", "[node][entity][image]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityImage>(nullptr);
    entity->m_imageScaleType = ImageScaleType::NineSlice;

    auto node = std::make_shared<NodeImage>(mc.context, entity);
    REQUIRE(node->GetImageScaleType() == ImageScaleType::NineSlice);
}

TEST_CASE("NodeImage reads image scale from LayoutEntityImage on construction", "[node][entity][image]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityImage>(nullptr);
    entity->m_imageScale = 2.5f;

    auto node = std::make_shared<NodeImage>(mc.context, entity);
    REQUIRE(node->GetImageScale() == Catch::Approx(2.5f));
}

TEST_CASE("NodeImage with missing image path leaves image null", "[node][entity][image]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityImage>(nullptr);
    // m_imagePath left empty — MockImageFactory returns nullptr

    auto node = std::make_shared<NodeImage>(mc.context, entity);
    REQUIRE(node->GetImage() == nullptr);
}
