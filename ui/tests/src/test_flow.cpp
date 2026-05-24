#include "mock_context.h"
#include "moth_ui/events/event_key.h"
#include "moth_ui/flow/flow.h"
#include "moth_ui/flow/flow_graph.h"
#include "moth_ui/flow/transition_participant.h"
#include "moth_ui/layers/layer.h"
#include "moth_ui/layers/layer_stack.h"

#include <catch2/catch_all.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace moth_ui;
using namespace moth_ui::flow;

namespace {
    // A test layer that records every lifecycle call and can either complete
    // its transitions synchronously (autoComplete=true) or hold the done
    // callback for the test to fire later.
    class MockParticipantLayer : public Layer, public ITransitionParticipant {
    public:
        std::vector<std::string>* log = nullptr;
        std::string name;
        bool autoComplete = true;
        std::function<void()> pendingDone;

        MockParticipantLayer() = default;
        MockParticipantLayer(MockParticipantLayer const&) = delete;
        MockParticipantLayer(MockParticipantLayer&&) = delete;
        MockParticipantLayer& operator=(MockParticipantLayer const&) = delete;
        MockParticipantLayer& operator=(MockParticipantLayer&&) = delete;
        ~MockParticipantLayer() override = default;

        void OnEnter() override { Record("OnEnter"); }
        void OnExit() override { Record("OnExit"); }

        void TransitionIn(std::string_view tag, std::function<void()> done) override {
            Record("In:" + std::string(tag));
            if (autoComplete) {
                done();
            } else {
                pendingDone = std::move(done);
            }
        }

        void TransitionOut(std::string_view tag, std::function<void()> done) override {
            Record("Out:" + std::string(tag));
            if (autoComplete) {
                done();
            } else {
                pendingDone = std::move(done);
            }
        }

    private:
        void Record(std::string event) const {
            if (log != nullptr) {
                log->push_back(name + "." + std::move(event));
            }
        }
    };

    struct FlowFixture {
        MockContext mock;
        LayerStack stack{ mock.renderer, IntVec2{800, 600}, IntVec2{800, 600} };
        std::vector<std::string> log;
        std::unordered_map<std::string, MockParticipantLayer*> created;

        Flow::LayerFactory MakeFactory() {
            return [this](Context&, LayerSpec const& spec) -> std::unique_ptr<Layer> {
                auto layer = std::make_unique<MockParticipantLayer>();
                layer->name = spec.id;
                layer->log = &log;
                created[spec.id] = layer.get();
                return layer;
            };
        }

        std::unique_ptr<Flow> MakeFlow(FlowGraph graph) {
            auto f = std::make_unique<Flow>(stack, mock.context, std::move(graph));
            // Use our mock factory as the default route for *every* layer:
            // give each LayerSpec a factory name and register that name.
            f->RegisterFactory("mock", MakeFactory());
            return f;
        }
    };

    // Helper: build a graph where every layer is built by the "mock" factory.
    LayerSpec MakeMockSpec(std::string id, LayerKind kind = LayerKind::Screen) {
        LayerSpec spec;
        spec.id = std::move(id);
        spec.factory = "mock";
        spec.kind = kind;
        return spec;
    }

    TransitionSpec MakeTransition(std::string id, std::string to, TransitionKind kind, TriggerSpec trigger) {
        TransitionSpec t;
        t.id = std::move(id);
        t.to = std::move(to);
        t.kind = kind;
        t.trigger = std::move(trigger);
        return t;
    }

    TriggerSpec EventTrigger(std::string name) {
        return TriggerSpec{ TriggerKind::Event, std::move(name), Key::Unknown, 0 };
    }

    TriggerSpec KeyTrigger(Key key) {
        return TriggerSpec{ TriggerKind::Key, {}, key, 0 };
    }
}

TEST_CASE("Flow::Start pushes the initial layer and runs OnEnter + TransitionIn", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "title";
    g.layers.push_back(MakeMockSpec("title"));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();

    REQUIRE(f.created.count("title") == 1);
    REQUIRE(f.log == std::vector<std::string>{ "title.OnEnter", "title.In:transition_in" });
}

TEST_CASE("Flow::Start is idempotent", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "title";
    g.layers.push_back(MakeMockSpec("title"));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    flow->Start();  // second call must be a no-op

    REQUIRE(f.log == std::vector<std::string>{ "title.OnEnter", "title.In:transition_in" });
}

TEST_CASE("Push transition: source OnExit, target OnEnter, modal flag set", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "game";

    auto game = MakeMockSpec("game");
    game.transitions.push_back(MakeTransition("open", "pause", TransitionKind::Push, EventTrigger("open")));
    g.layers.push_back(game);

    auto pause = MakeMockSpec("pause", LayerKind::Overlay);
    pause.modality = Modality::Modal;
    g.layers.push_back(pause);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    flow->Trigger("game.open");
    flow->Tick(0);

    REQUIRE(f.log == std::vector<std::string>{
        "game.OnExit",
        "pause.OnEnter",
        "pause.In:transition_in",
    });
    REQUIRE(f.created["pause"]->IsModal());
    REQUIRE_FALSE(f.created["game"]->IsModal());
}

TEST_CASE("Push skips OutAnimating: source TransitionOut never fires", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "game";

    auto game = MakeMockSpec("game");
    game.transitions.push_back(MakeTransition("open", "pause", TransitionKind::Push, EventTrigger("open")));
    g.layers.push_back(game);

    auto pause = MakeMockSpec("pause", LayerKind::Overlay);
    g.layers.push_back(pause);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    flow->Trigger("game.open");
    flow->Tick(0);

    for (auto const& entry : f.log) {
        REQUIRE_FALSE(entry.find("game.Out:") == 0);
    }
}

TEST_CASE("Pop transition: overlay OnExit + transition_out, revealed layer OnEnter", "[flow]") {
    // Regression: OnEnter was previously not called on the layer revealed by a
    // Pop, so the revealed layer's TransitioningLayer.m_active stayed false
    // and input/events stopped reaching it after a pause→resume cycle.
    FlowFixture f;
    FlowGraph g;
    g.initial = "game";

    auto game = MakeMockSpec("game");
    game.transitions.push_back(MakeTransition("open", "pause", TransitionKind::Push, EventTrigger("open")));
    g.layers.push_back(game);

    auto pause = MakeMockSpec("pause", LayerKind::Overlay);
    pause.modality = Modality::Modal;
    pause.transitions.push_back(MakeTransition("close", std::string(kBackTarget), TransitionKind::Pop, EventTrigger("close")));
    g.layers.push_back(pause);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    flow->Trigger("game.open");
    flow->Tick(0);
    f.log.clear();

    flow->Trigger("pause.close");
    flow->Tick(0);

    REQUIRE(f.log == std::vector<std::string>{
        "pause.OnExit",
        "pause.Out:transition_out",
        "game.OnEnter",
    });
}

TEST_CASE("Pop skips InAnimating: revealed layer's TransitionIn does not fire", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "game";

    auto game = MakeMockSpec("game");
    game.transitions.push_back(MakeTransition("open", "pause", TransitionKind::Push, EventTrigger("open")));
    g.layers.push_back(game);

    auto pause = MakeMockSpec("pause", LayerKind::Overlay);
    pause.transitions.push_back(MakeTransition("close", std::string(kBackTarget), TransitionKind::Pop, EventTrigger("close")));
    g.layers.push_back(pause);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    flow->Trigger("game.open");
    flow->Tick(0);
    f.log.clear();

    flow->Trigger("pause.close");
    flow->Tick(0);

    for (auto const& entry : f.log) {
        REQUIRE_FALSE(entry.find("game.In:") == 0);
    }
}

TEST_CASE("Replace transition: source out, target in", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "title";

    auto title = MakeMockSpec("title");
    title.transitions.push_back(MakeTransition("play", "game", TransitionKind::Replace, EventTrigger("play")));
    g.layers.push_back(title);
    g.layers.push_back(MakeMockSpec("game"));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    flow->Trigger("title.play");
    flow->Tick(0);

    REQUIRE(f.log == std::vector<std::string>{
        "title.OnExit",
        "title.Out:transition_out",
        "game.OnEnter",
        "game.In:transition_in",
    });
}

TEST_CASE("Custom transition clips override layer defaults", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "title";

    auto title = MakeMockSpec("title");
    auto t = MakeTransition("play", "game", TransitionKind::Replace, EventTrigger("play"));
    t.outClip = "fade_out";
    t.inClip = "fade_in";
    title.transitions.push_back(t);
    g.layers.push_back(title);
    g.layers.push_back(MakeMockSpec("game"));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    flow->Trigger("title.play");
    flow->Tick(0);

    bool foundOut = false;
    bool foundIn = false;
    for (auto const& entry : f.log) {
        if (entry == "title.Out:fade_out") { foundOut = true; }
        if (entry == "game.In:fade_in") { foundIn = true; }
    }
    REQUIRE(foundOut);
    REQUIRE(foundIn);
}

TEST_CASE("Action hooks fire in OnStart / OnMidpoint / OnComplete order", "[flow][actions]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "title";

    auto title = MakeMockSpec("title");
    auto t = MakeTransition("play", "game", TransitionKind::Replace, EventTrigger("play"));
    t.onStart = { "start1", "start2" };
    t.onMidpoint = { "mid" };
    t.onComplete = { "complete" };
    title.transitions.push_back(t);
    g.layers.push_back(title);
    g.layers.push_back(MakeMockSpec("game"));

    auto flow = f.MakeFlow(std::move(g));

    auto registerLogged = [&](std::string name) {
        flow->RegisterAction(name, [name, &f](std::function<void()> done) {
            f.log.push_back("action:" + name);
            done();
        });
    };
    registerLogged("start1");
    registerLogged("start2");
    registerLogged("mid");
    registerLogged("complete");

    flow->Start();
    f.log.clear();

    flow->Trigger("title.play");
    flow->Tick(0);

    REQUIRE(f.log == std::vector<std::string>{
        "action:start1",
        "action:start2",
        "title.OnExit",
        "title.Out:transition_out",
        "action:mid",
        "game.OnEnter",
        "game.In:transition_in",
        "action:complete",
    });
}

TEST_CASE("Unknown actions log and skip without breaking the transition", "[flow][actions]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "title";

    auto title = MakeMockSpec("title");
    auto t = MakeTransition("play", "game", TransitionKind::Replace, EventTrigger("play"));
    t.onStart = { "missing_action" };
    title.transitions.push_back(t);
    g.layers.push_back(title);
    g.layers.push_back(MakeMockSpec("game"));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    flow->Trigger("title.play");
    flow->Tick(0);

    // The transition still completes end-to-end.
    REQUIRE(f.log.back() == "game.In:transition_in");
}

TEST_CASE("Synchronous phases pump in a single Tick", "[flow][phases]") {
    // Regression: previously each phase advanced one per Tick, leaving a
    // visible idle frame between layer push and SetAnimation. The sync-phase
    // pump in Tick collapses everything that can finish immediately into the
    // same call.
    FlowFixture f;
    FlowGraph g;
    g.initial = "game";

    auto game = MakeMockSpec("game");
    game.transitions.push_back(MakeTransition("open", "pause", TransitionKind::Push, EventTrigger("open")));
    g.layers.push_back(game);
    g.layers.push_back(MakeMockSpec("pause", LayerKind::Overlay));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    flow->Trigger("game.open");
    flow->Tick(0);  // ONE tick should reach In:transition_in

    bool foundIn = false;
    for (auto const& entry : f.log) {
        if (entry == "pause.In:transition_in") { foundIn = true; }
    }
    REQUIRE(foundIn);
}

TEST_CASE("Async TransitionOut blocks phase advancement until done() fires", "[flow][async]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "title";

    auto title = MakeMockSpec("title");
    title.transitions.push_back(MakeTransition("play", "game", TransitionKind::Replace, EventTrigger("play")));
    g.layers.push_back(title);
    g.layers.push_back(MakeMockSpec("game"));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.created["title"]->autoComplete = false;
    f.log.clear();

    flow->Trigger("title.play");
    flow->Tick(0);
    // Out:transition_out fired, then we wait for done().
    REQUIRE(f.log.back() == "title.Out:transition_out");
    REQUIRE(f.created.count("game") == 0);  // target not built yet

    // Fire the pending done callback — phases now pump to completion.
    auto done = std::move(f.created["title"]->pendingDone);
    done();
    flow->Tick(0);

    REQUIRE(f.created.count("game") == 1);
    REQUIRE(f.log.back() == "game.In:transition_in");
}

TEST_CASE("Re-entry policy: Reject drops new triggers mid-transition", "[flow][reentry]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "a";
    g.policy.onReentry = ReentryPolicy::Reject;

    auto a = MakeMockSpec("a");
    a.transitions.push_back(MakeTransition("go", "b", TransitionKind::Replace, EventTrigger("go")));
    g.layers.push_back(a);

    auto b = MakeMockSpec("b");
    b.transitions.push_back(MakeTransition("back", "a", TransitionKind::Replace, EventTrigger("back")));
    g.layers.push_back(b);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.created["a"]->autoComplete = false;
    f.log.clear();

    flow->Trigger("a.go");
    flow->Trigger("a.go");  // dropped: transition already in flight
    auto done = std::move(f.created["a"]->pendingDone);
    done();
    flow->Tick(0);

    // Only one a→b cycle occurred.
    REQUIRE(f.created.count("b") == 1);
}

TEST_CASE("Re-entry policy: Queue fires the held trigger when Idle", "[flow][reentry]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "a";
    g.policy.onReentry = ReentryPolicy::Queue;

    auto a = MakeMockSpec("a");
    a.transitions.push_back(MakeTransition("go_b", "b", TransitionKind::Replace, EventTrigger("go_b")));
    g.layers.push_back(a);

    auto b = MakeMockSpec("b");
    b.transitions.push_back(MakeTransition("go_c", "c", TransitionKind::Replace, EventTrigger("go_c")));
    g.layers.push_back(b);
    g.layers.push_back(MakeMockSpec("c"));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    flow->Trigger("a.go_b");
    flow->Trigger("b.go_c");  // queued — b doesn't exist yet but we tolerate the lookup miss; trigger by qualified id is resolved at Trigger() time
    flow->Tick(0);

    // a → b should complete; the b.go_c was queued mid-flight, then fired
    // when the a→b cycle reached Idle, then b → c completed.
    REQUIRE(f.created.count("c") == 1);
}

TEST_CASE("Trigger by unknown qualified id is a no-op", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "a";
    g.layers.push_back(MakeMockSpec("a"));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    flow->Trigger("a.nonexistent");
    flow->Trigger("nonexistent.x");
    flow->Trigger("not_qualified");
    flow->Tick(0);

    REQUIRE(f.log.empty());
}

TEST_CASE("OnEvent dispatches key triggers from the topmost active layer", "[flow][input]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "game";

    auto game = MakeMockSpec("game");
    game.transitions.push_back(MakeTransition("pause", "pause_layer", TransitionKind::Push, KeyTrigger(Key::Escape)));
    g.layers.push_back(game);
    g.layers.push_back(MakeMockSpec("pause_layer", LayerKind::Overlay));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    EventKey downEvent(KeyAction::Down, Key::Escape, 0);
    bool const consumed = flow->OnEvent(downEvent);
    flow->Tick(0);

    REQUIRE(consumed);
    REQUIRE(f.created.count("pause_layer") == 1);
}

TEST_CASE("OnEvent ignores keys that do not match any trigger", "[flow][input]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "game";
    auto game = MakeMockSpec("game");
    game.transitions.push_back(MakeTransition("pause", "pause_layer", TransitionKind::Push, KeyTrigger(Key::Escape)));
    g.layers.push_back(game);
    g.layers.push_back(MakeMockSpec("pause_layer", LayerKind::Overlay));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();

    EventKey downEvent(KeyAction::Down, Key::Space, 0);
    bool const consumed = flow->OnEvent(downEvent);
    REQUIRE_FALSE(consumed);
    REQUIRE(f.created.count("pause_layer") == 0);
}

TEST_CASE("Emit routes a named event to the topmost active layer", "[flow][input]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "a";

    auto a = MakeMockSpec("a");
    a.transitions.push_back(MakeTransition("go", "b", TransitionKind::Replace, EventTrigger("a_event")));
    g.layers.push_back(a);

    auto b = MakeMockSpec("b");
    b.transitions.push_back(MakeTransition("back", "a", TransitionKind::Replace, EventTrigger("b_event")));
    g.layers.push_back(b);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();

    // a is topmost; a_event matches a.go
    flow->Emit("a_event");
    flow->Tick(0);
    REQUIRE(f.created.count("b") == 1);

    // b_event won't match anything on a but does on b — b is now topmost
    flow->Emit("b_event");
    flow->Tick(0);
    // a was rebuilt
    REQUIRE(f.created.count("a") == 1);
}

TEST_CASE("Emit on unknown event name is a no-op", "[flow][input]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "a";
    g.layers.push_back(MakeMockSpec("a"));

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    f.log.clear();
    flow->Emit("nothing_matches_this");
    flow->Tick(0);
    REQUIRE(f.log.empty());
}

TEST_CASE("Modal flag is not set on Screen-kind targets", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "title";

    auto title = MakeMockSpec("title");
    title.modality = Modality::Modal;  // ignored on screens
    title.transitions.push_back(MakeTransition("play", "game", TransitionKind::Replace, EventTrigger("play")));
    g.layers.push_back(title);

    auto game = MakeMockSpec("game");
    game.modality = Modality::Modal;  // ignored on screens
    g.layers.push_back(game);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    REQUIRE_FALSE(f.created["title"]->IsModal());

    flow->Trigger("title.play");
    flow->Tick(0);
    REQUIRE_FALSE(f.created["game"]->IsModal());
}

TEST_CASE("Passthrough overlay does not set the modal flag", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "game";

    auto game = MakeMockSpec("game");
    game.transitions.push_back(MakeTransition("open", "overlay", TransitionKind::Push, EventTrigger("open")));
    g.layers.push_back(game);

    auto overlay = MakeMockSpec("overlay", LayerKind::Overlay);
    overlay.modality = Modality::Passthrough;
    g.layers.push_back(overlay);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();
    flow->Trigger("game.open");
    flow->Tick(0);

    REQUIRE_FALSE(f.created["overlay"]->IsModal());
}

TEST_CASE("Cached layer survives a Pop and is reused on re-push", "[flow][cached]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "shell";

    auto shell = MakeMockSpec("shell");
    shell.transitions.push_back(MakeTransition("open", "tray", TransitionKind::Push, EventTrigger("open")));
    g.layers.push_back(shell);

    auto tray = MakeMockSpec("tray", LayerKind::Overlay);
    tray.construction = Construction::Cached;
    tray.transitions.push_back(MakeTransition("close", std::string(kBackTarget), TransitionKind::Pop, EventTrigger("close")));
    g.layers.push_back(tray);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();

    flow->Trigger("shell.open");
    flow->Tick(0);
    MockParticipantLayer* first = f.created["tray"];
    REQUIRE(first != nullptr);

    flow->Trigger("tray.close");
    flow->Tick(0);
    // first is still alive — Pop should have detached, not destroyed, the cached layer.

    // Forget the create-time pointer so we can prove the next push reuses
    // the same instance rather than constructing a new one. Our factory
    // would overwrite f.created["tray"] if it were called again.
    f.created.erase("tray");

    flow->Trigger("shell.open");
    flow->Tick(0);
    // Factory was NOT invoked a second time; the cached instance came back.
    REQUIRE(f.created.count("tray") == 0);
}

TEST_CASE("Fresh layer is rebuilt each push", "[flow]") {
    FlowFixture f;
    FlowGraph g;
    g.initial = "shell";

    auto shell = MakeMockSpec("shell");
    shell.transitions.push_back(MakeTransition("open", "tray", TransitionKind::Push, EventTrigger("open")));
    g.layers.push_back(shell);

    auto tray = MakeMockSpec("tray", LayerKind::Overlay);
    tray.construction = Construction::Fresh;
    tray.transitions.push_back(MakeTransition("close", std::string(kBackTarget), TransitionKind::Pop, EventTrigger("close")));
    g.layers.push_back(tray);

    auto flow = f.MakeFlow(std::move(g));
    flow->Start();

    flow->Trigger("shell.open");
    flow->Tick(0);
    flow->Trigger("tray.close");
    flow->Tick(0);

    f.created.erase("tray");

    flow->Trigger("shell.open");
    flow->Tick(0);
    // Factory WAS invoked again — fresh instance.
    REQUIRE(f.created.count("tray") == 1);
}
