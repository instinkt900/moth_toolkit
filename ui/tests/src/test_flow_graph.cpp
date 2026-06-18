#include "moth_ui/flow/flow_graph.h"

#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>

using namespace moth_ui;
using namespace moth_ui::flow;
using nlohmann::json;

namespace {
    json MinimalGraphJson() {
        return json::parse(R"({
            "initial": "a",
            "layers": [
                { "id": "a", "kind": "screen" }
            ]
        })");
    }

    LoadResult LoadOk(json const& j) {
        auto result = LoadFlowGraphFromJson(j);
        REQUIRE(result.ok());
        return result;
    }

    bool HasError(LoadResult const& r, std::string const& fragment) {
        for (auto const& e : r.errors) {
            if (e.message.find(fragment) != std::string::npos) {
                return true;
            }
        }
        return false;
    }
}

TEST_CASE("Loader rejects non-object root", "[flow_graph]") {
    auto result = LoadFlowGraphFromJson(json::array());
    REQUIRE_FALSE(result.ok());
    REQUIRE(HasError(result, "root JSON value must be an object"));
}

TEST_CASE("Loader requires 'initial'", "[flow_graph]") {
    auto j = MinimalGraphJson();
    j.erase("initial");
    auto result = LoadFlowGraphFromJson(j);
    REQUIRE_FALSE(result.ok());
    REQUIRE(HasError(result, "missing 'initial'"));
}

TEST_CASE("Loader requires 'layers'", "[flow_graph]") {
    auto j = MinimalGraphJson();
    j.erase("layers");
    auto result = LoadFlowGraphFromJson(j);
    REQUIRE_FALSE(result.ok());
    REQUIRE(HasError(result, "missing 'layers'"));
}

TEST_CASE("Minimal graph parses cleanly", "[flow_graph]") {
    auto result = LoadOk(MinimalGraphJson());
    auto const& g = *result.graph;
    REQUIRE(g.initial == "a");
    REQUIRE(g.layers.size() == 1);
    REQUIRE(g.layers[0].id == "a");
    REQUIRE(g.layers[0].kind == LayerKind::Screen);
    REQUIRE(g.layers[0].defaultInClip == "transition_in");
    REQUIRE(g.layers[0].defaultOutClip == "transition_out");
}

TEST_CASE("Policy parses with defaults and overrides", "[flow_graph]") {
    SECTION("defaults") {
        auto result = LoadOk(MinimalGraphJson());
        REQUIRE(result.graph->policy.onReentry == ReentryPolicy::Queue);
        REQUIRE(result.graph->policy.actionTimeoutMs == kDefaultActionTimeoutMs);
    }
    SECTION("explicit values") {
        constexpr int kOverrideTimeoutMs = 1000;
        auto j = MinimalGraphJson();
        j["policy"] = { { "onReentry", "reject" }, { "actionTimeoutMs", kOverrideTimeoutMs } };
        auto result = LoadOk(j);
        REQUIRE(result.graph->policy.onReentry == ReentryPolicy::Reject);
        REQUIRE(result.graph->policy.actionTimeoutMs == kOverrideTimeoutMs);
    }
}

TEST_CASE("LayerSpec parses optional fields", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "game",
        "layers": [
            {
                "id": "game",
                "factory": "myFactory",
                "layout": "assets/x.mothui",
                "kind": "screen",
                "construction": "cached",
                "defaultInClip": "in_x",
                "defaultOutClip": "out_x"
            }
        ]
    })");
    auto result = LoadOk(j);
    auto const& layer = result.graph->layers[0];
    REQUIRE(layer.factory.value() == "myFactory");
    REQUIRE(layer.layout.value() == "assets/x.mothui");
    REQUIRE(layer.construction == Construction::Cached);
    REQUIRE(layer.defaultInClip == "in_x");
    REQUIRE(layer.defaultOutClip == "out_x");
}

TEST_CASE("LayerSpec parses overlay modality", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "screen" },
            { "id": "b", "kind": "overlay", "modality": "passthrough" }
        ]
    })");
    auto result = LoadOk(j);
    REQUIRE(result.graph->layers[1].kind == LayerKind::Overlay);
    REQUIRE(result.graph->layers[1].modality == Modality::Passthrough);
}

TEST_CASE("Enum strings are case-insensitive", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "Screen", "construction": "FRESH" }
        ]
    })");
    auto result = LoadOk(j);
    REQUIRE(result.graph->layers[0].kind == LayerKind::Screen);
    REQUIRE(result.graph->layers[0].construction == Construction::Fresh);
}

TEST_CASE("Unknown enum values are errors", "[flow_graph]") {
    SECTION("layer kind") {
        auto j = MinimalGraphJson();
        j["layers"][0]["kind"] = "elephant";
        auto result = LoadFlowGraphFromJson(j);
        REQUIRE_FALSE(result.ok());
        REQUIRE(HasError(result, "unknown kind"));
    }
    SECTION("modality") {
        auto j = MinimalGraphJson();
        j["layers"][0]["modality"] = "potato";
        auto result = LoadFlowGraphFromJson(j);
        REQUIRE_FALSE(result.ok());
        REQUIRE(HasError(result, "unknown modality"));
    }
}

TEST_CASE("Button trigger parses", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "screen", "transitions": [
                { "id": "play", "to": "a", "kind": "replace",
                  "trigger": { "type": "button", "id": "btn_play" } }
            ]}
        ]
    })");
    auto result = LoadOk(j);
    auto const& t = result.graph->layers[0].transitions[0];
    REQUIRE(t.trigger.kind == TriggerKind::Button);
    REQUIRE(t.trigger.id == "btn_play");
}

TEST_CASE("Key trigger parses with named key", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "screen", "transitions": [
                { "id": "pause", "to": "a", "kind": "replace",
                  "trigger": { "type": "key", "key": "Escape" } }
            ]}
        ]
    })");
    auto result = LoadOk(j);
    REQUIRE(result.graph->layers[0].transitions[0].trigger.key == Key::Escape);
}

TEST_CASE("Unknown key name is an error", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "screen", "transitions": [
                { "id": "x", "to": "a", "kind": "replace",
                  "trigger": { "type": "key", "key": "FooBar" } }
            ]}
        ]
    })");
    auto result = LoadFlowGraphFromJson(j);
    REQUIRE_FALSE(result.ok());
    REQUIRE(HasError(result, "unknown key"));
}

TEST_CASE("Event trigger parses 'name' field", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "screen", "transitions": [
                { "id": "x", "to": "a", "kind": "replace",
                  "trigger": { "type": "event", "name": "match_ended" } }
            ]}
        ]
    })");
    auto result = LoadOk(j);
    auto const& t = result.graph->layers[0].transitions[0];
    REQUIRE(t.trigger.kind == TriggerKind::Event);
    REQUIRE(t.trigger.id == "match_ended");
}

TEST_CASE("Auto trigger parses afterMs", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "screen", "transitions": [
                { "id": "auto", "to": "a", "kind": "replace",
                  "trigger": { "type": "auto", "afterMs": 1500 } }
            ]}
        ]
    })");
    auto result = LoadOk(j);
    REQUIRE(result.graph->layers[0].transitions[0].trigger.afterMs == 1500);
}

TEST_CASE("Unknown trigger type is an error", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "screen", "transitions": [
                { "id": "x", "to": "a", "kind": "replace",
                  "trigger": { "type": "telepathy" } }
            ]}
        ]
    })");
    auto result = LoadFlowGraphFromJson(j);
    REQUIRE_FALSE(result.ok());
    REQUIRE(HasError(result, "unknown type"));
}

TEST_CASE("Transition clip overrides parse", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "screen", "transitions": [
                { "id": "x", "to": "a", "kind": "replace",
                  "outClip": "custom_out", "inClip": "custom_in",
                  "trigger": { "type": "event", "name": "e" } }
            ]}
        ]
    })");
    auto result = LoadOk(j);
    auto const& t = result.graph->layers[0].transitions[0];
    REQUIRE(t.outClip.value() == "custom_out");
    REQUIRE(t.inClip.value() == "custom_in");
}

TEST_CASE("Action hook lists parse in order", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "a",
        "layers": [
            { "id": "a", "kind": "screen", "transitions": [
                { "id": "x", "to": "a", "kind": "replace",
                  "trigger": { "type": "event", "name": "e" },
                  "onStart": ["a1", "a2"],
                  "onMidpoint": ["b1"],
                  "onComplete": ["c1", "c2", "c3"]
                }
            ]}
        ]
    })");
    auto result = LoadOk(j);
    auto const& t = result.graph->layers[0].transitions[0];
    REQUIRE(t.onStart == std::vector<ActionRef>{ "a1", "a2" });
    REQUIRE(t.onMidpoint == std::vector<ActionRef>{ "b1" });
    REQUIRE(t.onComplete == std::vector<ActionRef>{ "c1", "c2", "c3" });
}

TEST_CASE("FlowGraph::FindLayer", "[flow_graph]") {
    FlowGraph g;
    LayerSpec a; a.id = "a"; g.layers.push_back(a);
    LayerSpec b; b.id = "b"; g.layers.push_back(b);

    REQUIRE(g.FindLayer("a") != nullptr);
    REQUIRE(g.FindLayer("a")->id == "a");
    REQUIRE(g.FindLayer("b") != nullptr);
    REQUIRE(g.FindLayer("c") == nullptr);
}

TEST_CASE("Validator catches missing initial", "[flow_graph][validation]") {
    FlowGraph g;
    g.initial = "missing";
    LayerSpec a; a.id = "a"; g.layers.push_back(a);
    auto errors = ValidateFlowGraph(g);
    REQUIRE(!errors.empty());
}

TEST_CASE("Validator catches empty initial", "[flow_graph][validation]") {
    FlowGraph g;
    LayerSpec a; a.id = "a"; g.layers.push_back(a);
    auto errors = ValidateFlowGraph(g);
    REQUIRE(!errors.empty());
}

TEST_CASE("Validator catches duplicate layer ids", "[flow_graph][validation]") {
    FlowGraph g;
    g.initial = "a";
    LayerSpec a; a.id = "a"; g.layers.push_back(a);
    LayerSpec a2; a2.id = "a"; g.layers.push_back(a2);
    auto errors = ValidateFlowGraph(g);
    bool found = false;
    for (auto const& e : errors) {
        if (e.message.find("duplicate layer id") != std::string::npos) { found = true; }
    }
    REQUIRE(found);
}

TEST_CASE("Validator catches duplicate transition ids within layer", "[flow_graph][validation]") {
    FlowGraph g;
    g.initial = "a";
    LayerSpec a; a.id = "a";
    TransitionSpec t1; t1.id = "x"; t1.to = "a"; t1.kind = TransitionKind::Replace; a.transitions.push_back(t1);
    TransitionSpec t2; t2.id = "x"; t2.to = "a"; t2.kind = TransitionKind::Replace; a.transitions.push_back(t2);
    g.layers.push_back(a);
    auto errors = ValidateFlowGraph(g);
    bool found = false;
    for (auto const& e : errors) {
        if (e.message.find("duplicate transition id") != std::string::npos) { found = true; }
    }
    REQUIRE(found);
}

TEST_CASE("Validator: Pop must target <back>", "[flow_graph][validation]") {
    FlowGraph g;
    g.initial = "a";
    LayerSpec a; a.id = "a";
    TransitionSpec t; t.id = "x"; t.to = "a"; t.kind = TransitionKind::Pop;
    a.transitions.push_back(t);
    g.layers.push_back(a);
    auto errors = ValidateFlowGraph(g);
    bool found = false;
    for (auto const& e : errors) {
        if (e.message.find("Pop transitions must target '<back>'") != std::string::npos) { found = true; }
    }
    REQUIRE(found);
}

TEST_CASE("Validator: non-Pop may not target <back>", "[flow_graph][validation]") {
    FlowGraph g;
    g.initial = "a";
    LayerSpec a; a.id = "a";
    TransitionSpec t; t.id = "x"; t.to = std::string(kBackTarget); t.kind = TransitionKind::Replace;
    a.transitions.push_back(t);
    g.layers.push_back(a);
    auto errors = ValidateFlowGraph(g);
    bool found = false;
    for (auto const& e : errors) {
        if (e.message.find("only Pop transitions may target '<back>'") != std::string::npos) { found = true; }
    }
    REQUIRE(found);
}

TEST_CASE("Validator: unknown target layer is an error", "[flow_graph][validation]") {
    FlowGraph g;
    g.initial = "a";
    LayerSpec a; a.id = "a";
    TransitionSpec t; t.id = "x"; t.to = "ghost"; t.kind = TransitionKind::Replace;
    a.transitions.push_back(t);
    g.layers.push_back(a);
    auto errors = ValidateFlowGraph(g);
    bool found = false;
    for (auto const& e : errors) {
        if (e.message.find("unknown layer 'ghost'") != std::string::npos) { found = true; }
    }
    REQUIRE(found);
}

TEST_CASE("Validator: Push must target an Overlay", "[flow_graph][validation]") {
    FlowGraph g;
    g.initial = "a";
    LayerSpec a; a.id = "a"; a.kind = LayerKind::Screen;
    LayerSpec b; b.id = "b"; b.kind = LayerKind::Screen;  // target is Screen, not Overlay
    TransitionSpec t; t.id = "x"; t.to = "b"; t.kind = TransitionKind::Push;
    a.transitions.push_back(t);
    g.layers.push_back(a);
    g.layers.push_back(b);
    auto errors = ValidateFlowGraph(g);
    bool found = false;
    for (auto const& e : errors) {
        if (e.message.find("Push must target an Overlay") != std::string::npos) { found = true; }
    }
    REQUIRE(found);
}

TEST_CASE("Validator reports all errors at once, not just the first", "[flow_graph][validation]") {
    FlowGraph g;
    g.initial = "missing";
    LayerSpec a; a.id = "a";
    TransitionSpec t1; t1.id = "x"; t1.to = "ghost"; t1.kind = TransitionKind::Replace;
    a.transitions.push_back(t1);
    TransitionSpec t2; t2.id = "y"; t2.to = std::string(kBackTarget); t2.kind = TransitionKind::Replace;
    a.transitions.push_back(t2);
    g.layers.push_back(a);
    auto errors = ValidateFlowGraph(g);
    REQUIRE(errors.size() >= 3);  // initial missing, ghost target, <back> on non-pop
}

TEST_CASE("Loader runs the validator", "[flow_graph]") {
    auto j = json::parse(R"({
        "initial": "ghost",
        "layers": [ { "id": "a", "kind": "screen" } ]
    })");
    auto result = LoadFlowGraphFromJson(j);
    REQUIRE_FALSE(result.ok());
}
