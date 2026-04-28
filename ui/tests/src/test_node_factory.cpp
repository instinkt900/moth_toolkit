#include "mock_context.h"
#include "moth_ui/layout/layout_entity_clip.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/nodes/node_clip.h"
#include "moth_ui/nodes/node_image.h"
#include "moth_ui/nodes/node_rect.h"
#include "moth_ui/nodes/node_text.h"
#include "moth_ui/nodes/group.h"
#include <catch2/catch_all.hpp>

using namespace moth_ui;

TEST_CASE("NodeFactory Create returns NodeRect from LayoutEntityRect", "[nodefactory][create]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityRect>(LayoutRect{});
    auto node = NodeFactory::Get().Create(mc.context, entity);
    REQUIRE(node != nullptr);
    REQUIRE(dynamic_cast<NodeRect*>(node.get()) != nullptr);
}

TEST_CASE("NodeFactory Create returns NodeText from LayoutEntityText", "[nodefactory][create]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityText>(LayoutRect{});
    auto node = NodeFactory::Get().Create(mc.context, entity);
    REQUIRE(node != nullptr);
    REQUIRE(dynamic_cast<NodeText*>(node.get()) != nullptr);
}

TEST_CASE("NodeFactory Create returns NodeImage from LayoutEntityImage", "[nodefactory][create]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityImage>(LayoutRect{});
    auto node = NodeFactory::Get().Create(mc.context, entity);
    REQUIRE(node != nullptr);
    REQUIRE(dynamic_cast<NodeImage*>(node.get()) != nullptr);
}

TEST_CASE("NodeFactory Create returns NodeClip from LayoutEntityClip", "[nodefactory][create]") {
    MockContext mc;
    auto entity = std::make_shared<LayoutEntityClip>(LayoutRect{});
    auto node = NodeFactory::Get().Create(mc.context, entity);
    REQUIRE(node != nullptr);
    REQUIRE(dynamic_cast<NodeClip*>(node.get()) != nullptr);
}

TEST_CASE("NodeFactory RegisterWidget returns class name", "[nodefactory][register]") {
    auto name = NodeFactory::Get().RegisterWidget("TestWidget", [](Context& ctx, std::shared_ptr<LayoutEntityGroup> entity) -> std::shared_ptr<Group> {
        return Group::Create(ctx, std::move(entity));
    });
    REQUIRE(name == "TestWidget");
}
