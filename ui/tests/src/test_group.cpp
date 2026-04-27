#include "mock_context.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/nodes/node.h"
#include <catch2/catch_all.hpp>
#include <memory>

using namespace moth_ui;

TEST_CASE("Group default constructor allows Update without crashing", "[group][animation]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    REQUIRE_NOTHROW(group->Update(16));
}

TEST_CASE("Group default constructor provides a valid AnimationClipController", "[group][animation]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    REQUIRE_NOTHROW(group->StopAnimation());
}

TEST_CASE("Group default child count is zero", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    REQUIRE(group->GetChildCount() == 0);
    REQUIRE(group->GetChildren().empty());
}

TEST_CASE("Group AddChild increases child count", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto child = std::make_shared<Node>(mc.context);

    group->AddChild(child);
    REQUIRE(group->GetChildCount() == 1);
}

TEST_CASE("Group AddChild sets parent pointer", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto child = std::make_shared<Node>(mc.context);

    group->AddChild(child);
    REQUIRE(child->GetParent() == group.get());
}

TEST_CASE("Group AddChild with index inserts at correct position", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto a = std::make_shared<Node>(mc.context);
    auto b = std::make_shared<Node>(mc.context);
    auto c = std::make_shared<Node>(mc.context);

    group->AddChild(a);
    group->AddChild(b);
    group->AddChild(c, 1); // insert c between a and b

    auto& children = group->GetChildren();
    REQUIRE(children[0] == a);
    REQUIRE(children[1] == c);
    REQUIRE(children[2] == b);
}

TEST_CASE("Group RemoveChild decreases child count", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto child = std::make_shared<Node>(mc.context);

    group->AddChild(child);
    REQUIRE(group->GetChildCount() == 1);

    group->RemoveChild(child);
    REQUIRE(group->GetChildCount() == 0);
}

TEST_CASE("Group RemoveChild clears parent pointer", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto child = std::make_shared<Node>(mc.context);

    group->AddChild(child);
    group->RemoveChild(child);
    REQUIRE(child->GetParent() == nullptr);
}

TEST_CASE("Group IndexOf returns correct position", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto a = std::make_shared<Node>(mc.context);
    auto b = std::make_shared<Node>(mc.context);
    auto c = std::make_shared<Node>(mc.context);

    group->AddChild(a);
    group->AddChild(b);
    group->AddChild(c);

    REQUIRE(group->IndexOf(a) == 0);
    REQUIRE(group->IndexOf(b) == 1);
    REQUIRE(group->IndexOf(c) == 2);
}

TEST_CASE("Group GetChild finds by id", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto child = std::make_shared<Node>(mc.context);
    child->SetId("target");

    group->AddChild(child);
    auto found = group->GetChild("target");
    REQUIRE(found == child);
}

TEST_CASE("Group GetChild returns nullptr for unknown id", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    REQUIRE(group->GetChild("nonexistent") == nullptr);
}

TEST_CASE("Group FindChild searches recursively", "[group][children]") {
    MockContext mc;
    auto root  = std::make_shared<Group>(mc.context);
    auto inner = std::make_shared<Group>(mc.context);
    auto leaf  = std::make_shared<Node>(mc.context);
    leaf->SetId("deep");

    inner->AddChild(leaf);
    root->AddChild(inner);

    auto found = root->FindChild("deep");
    REQUIRE(found == leaf);
}

TEST_CASE("Group FindChild returns nullptr when not found", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    REQUIRE(group->FindChild("missing") == nullptr);
}

TEST_CASE("Group multiple children are ordered", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto a = std::make_shared<Node>(mc.context);
    auto b = std::make_shared<Node>(mc.context);
    auto c = std::make_shared<Node>(mc.context);

    group->AddChild(a);
    group->AddChild(b);
    group->AddChild(c);

    auto& children = group->GetChildren();
    REQUIRE(children.size() == 3);
    REQUIRE(children[0] == a);
    REQUIRE(children[1] == b);
    REQUIRE(children[2] == c);
}

TEST_CASE("Group inherits Node properties", "[group][node]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);

    group->SetId("my_group");
    group->SetVisible(false);

    REQUIRE(group->GetId() == "my_group");
    REQUIRE_FALSE(group->IsVisible());
}

TEST_CASE("Group MoveChild reorders children", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto a = std::make_shared<Node>(mc.context);
    auto b = std::make_shared<Node>(mc.context);
    auto c = std::make_shared<Node>(mc.context);

    group->AddChild(a);
    group->AddChild(b);
    group->AddChild(c);

    group->MoveChild(0, 2);

    auto const& children = group->GetChildren();
    REQUIRE(children[0] == b);
    REQUIRE(children[1] == c);
    REQUIRE(children[2] == a);
}

TEST_CASE("Group MoveChild forward preserves count", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto a = std::make_shared<Node>(mc.context);
    auto b = std::make_shared<Node>(mc.context);

    group->AddChild(a);
    group->AddChild(b);

    group->MoveChild(0, 1);

    REQUIRE(group->GetChildCount() == 2);
    auto const& children = group->GetChildren();
    REQUIRE(children[0] == b);
    REQUIRE(children[1] == a);
}

TEST_CASE("Group MoveChild same index is no-op", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto a = std::make_shared<Node>(mc.context);

    group->AddChild(a);
    REQUIRE_NOTHROW(group->MoveChild(0, 0));

    REQUIRE(group->GetChildCount() == 1);
    REQUIRE(group->GetChildren()[0] == a);
}

TEST_CASE("Group MoveChild out-of-range fromIndex is no-op", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto a = std::make_shared<Node>(mc.context);
    group->AddChild(a);

    REQUIRE_NOTHROW(group->MoveChild(-1, 0));
    REQUIRE_NOTHROW(group->MoveChild(99, 0));

    REQUIRE(group->GetChildren()[0] == a);
}

TEST_CASE("Group MoveChild out-of-range toIndex is no-op", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto a = std::make_shared<Node>(mc.context);
    group->AddChild(a);

    REQUIRE_NOTHROW(group->MoveChild(0, -1));
    REQUIRE_NOTHROW(group->MoveChild(0, 99));

    REQUIRE(group->GetChildren()[0] == a);
}

TEST_CASE("Group RemoveChild on non-member is no-op", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto child = std::make_shared<Node>(mc.context);
    auto other = std::make_shared<Node>(mc.context);

    group->AddChild(child);
    REQUIRE_NOTHROW(group->RemoveChild(other));
    REQUIRE(group->GetChildCount() == 1);
}

TEST_CASE("Group AddChild with index greater than size appends", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    auto a = std::make_shared<Node>(mc.context);
    auto b = std::make_shared<Node>(mc.context);

    group->AddChild(a);
    group->AddChild(b, 99);

    auto const& children = group->GetChildren();
    REQUIRE(children[0] == a);
    REQUIRE(children[1] == b);
}

TEST_CASE("Group GetChildren returns const reference from non-const object", "[group][children]") {
    MockContext mc;
    auto group = std::make_shared<Group>(mc.context);
    group->AddChild(std::make_shared<Node>(mc.context));

    // Verify that the returned reference can be read but not mutated via the const overload
    auto const& children = group->GetChildren();
    REQUIRE(children.size() == 1);
}
