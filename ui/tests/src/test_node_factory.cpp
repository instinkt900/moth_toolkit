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
#include "moth_ui/layout/layout.h"
#include <catch2/catch_all.hpp>

using namespace moth_ui;

namespace {
    bool s_registerWidgetInvoked = false;
}

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

TEST_CASE("NodeFactory RegisterWidget routes creation through registered factory", "[nodefactory][register]") {
    MockContext mc;
    s_registerWidgetInvoked = false;
    auto name = NodeFactory::Get().RegisterWidget("TestWidget_Factory", [](Context& ctx, std::shared_ptr<LayoutEntityGroup>) -> std::shared_ptr<Group> {
        s_registerWidgetInvoked = true;
        return Group::Create(ctx);
    });
    REQUIRE(name == "TestWidget_Factory");

    auto entity = std::make_shared<Layout>();
    entity->m_class = "TestWidget_Factory";
    auto result = NodeFactory::Get().Create(mc.context, std::static_pointer_cast<LayoutEntityGroup>(entity));
    REQUIRE(result != nullptr);
    REQUIRE(s_registerWidgetInvoked);
}
