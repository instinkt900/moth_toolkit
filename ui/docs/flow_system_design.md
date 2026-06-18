# Flow System — Design

Status: implemented (PR #147). The runtime, layer bases, JSON loader, and bundled
`UIButton` widget have all landed under `moth_ui/flow/` and `moth_ui/widgets/`. This
document is the **design-of-record** — it captures the rationale (why transitions
nest under layers, why the stack mutates before the midpoint, the string-`tag`
contract, deferred editor scope). It is written partly in pre-implementation tense
and describes scorched_moth's *old* hand-rolled flow code as the thing being
generalised; that code is gone now and scorched_moth consumes `moth_ui::flow`
directly.

**For a consumer-facing how-to** — standing up a `Flow`, the real API surface,
building a graph, and the gotchas — see
[`flow_system_guide.md`](flow_system_guide.md). Where this doc and the guide disagree
on concrete API, the guide and the headers win.

Location: moth_ui, as an opt-in utility. Existing moth_ui users with their own
layer/transition setup ignore the new headers and lose nothing; new users include
`<moth_ui/flow/...>` and pick it up. The system has no moth_graphics dependency — the
entire flow runtime, layer bases, and supporting widgets are pure moth_ui.

## Goal

A reusable navigation system for moth_ui-based applications. Apps define a graph of
layers (screens and overlays) and transitions between them. The framework runs the
transitions — playing animations, mutating the layer stack, sequencing side effects —
so that application code stays focused on the layer's own logic, not on plumbing.

The long-term goal is a visual editor that lets a designer build the flow graph
(layers + transitions + button bindings) without writing code. This design treats
that editor as the eventual primary consumer of the data model — every choice should
keep the graph declarative, serialisable, and statically validatable where possible.

## What the framework provides today, conceptually

Scorched_moth's `src/flow/` and `src/layers/ui_layer.{h,cpp}` already contain ~250
lines of working flow infrastructure that we intend to lift and generalise:

- A phase state machine (Idle / OutAnimating / InAnimating) coordinating transitions
  between two layers.
- Deferred push/pop queues drained at the start of each `Tick`, so callers can mutate
  the stack from inside event handlers or `Update` without invalidating LayerStack
  iterators.
- Overlay-vs-screen distinction with a "current screen" slot and an overlay vector
  layered on top.
- A double-pop guard (`m_exitingOverlays`) preventing a rapid second `PopOverlay`
  from restarting an out-animation.
- A clip-driven base layer that loads a `.mothui` layout, plays `transition_in` /
  `transition_out` clips, and converts the layer-wide `EventAnimationStopped` event
  into a `done()` callback firing.
- Input gating via an `m_active` flag so layers don't receive events during
  transitions.

The current code conflates screen identity (`enum class Screen { Title, Game }`), the
layer factory (`CreateLayerFor` in `flow.cpp`), and the transition mechanics. The
redesign pulls those apart: the framework owns the mechanics, the application owns
the identities and factories, and the relationships between layers move into a
serialisable graph.

Scorched_moth also defines a `UIButton` widget (a moth_ui `Widget<T>` CRTP subclass)
that exposes a click action. That widget moves into moth_ui alongside the flow system
as a generally useful opt-in primitive — see "Buttons and the IClickable interface"
below.

## Core model

### FlowGraph

The serialisable artifact. An app loads a graph at startup, hands it to a `Flow`
runtime, and from that point on, the graph drives navigation.

```text
FlowGraph {
    initial:   LayerId
    policy:    GraphPolicy
    layers:    LayerSpec[]
}

GraphPolicy {
    onReentry: Reject | Queue | Coalesce   # default Queue depth 1
    actionTimeoutMs: int                   # default several seconds, framework default
}
```

### LayerSpec

One node in the graph. Carries everything the runtime needs to construct and animate
this layer, plus the outgoing transitions originating from it.

```text
LayerSpec {
    id:               LayerId          # unique within graph, e.g. "title", "game", "pause"
    factory:          string | null    # name of an app-registered factory; null → default
    layout:           string | null    # path to .mothui, consumed by default factory
    kind:             Screen | Overlay
    modality:         Modal | Passthrough   # overlays only; controls input/update of layers below
    construction:     Fresh | Cached        # rebuild on entry, or preserve instance across visits
    defaultOutClip:   string                # fallback used when a transition doesn't override
    defaultInClip:    string                # fallback used when a transition doesn't override
    transitions:      TransitionSpec[]      # outgoing edges from this layer
}
```

Transitions are nested under their source layer rather than living in a flat list
with a `from` field. Rationale:

- Designers think of transitions as "from this screen I can go to..." — the JSON
  structure should mirror that.
- The editor's natural per-layer card representation maps directly to nested
  transitions.
- Lookup at runtime is cheaper: resolving "what does this button do here" walks
  `currentLayer.transitions` instead of filtering the whole transition table by `from
  == current`.
- Validation can attribute errors to a specific source layer naturally.

The trade-off: cross-cutting transitions (e.g., "from any layer, Esc → settings
overlay") become awkward; you have to repeat the entry on every applicable layer. The
editor handles that as authoring sugar (multi-select or a "global transitions" panel
that expands on save). The runtime stays simple.

### TransitionSpec

One outgoing edge. Locally addressable as `"layerId.transitionId"`.

```text
TransitionSpec {
    id:        string              # local to parent layer; readable as "title.play"
    to:        LayerId | "<back>"  # "<back>" pops the topmost overlay
    kind:      Replace | Push | Pop
    outClip:   string | null       # null → parent layer's defaultOutClip
    inClip:    string | null       # null → target layer's defaultInClip
    trigger:   TriggerSpec
    onStart:   ActionRef[]         # side effects, see "Side effects" below
    onMidpoint:ActionRef[]
    onComplete:ActionRef[]
}
```

The clip names are owned by the source-side transition. A transition's `inClip`
therefore references a clip defined on the *target* layer's layout. This is a
deliberate asymmetry: the choice of *which* in-clip to play is a property of the edge
(the visual moment), not a property of the target layer in general. The target's
`defaultInClip` is the fallback for transitions that don't care.

The graph loader can validate `inClip` references at load time by reading the
target's layout — fail loudly if a referenced clip doesn't exist. The editor can
drive the same validation interactively (the inClip dropdown enumerates the target
layout's clips).

### TriggerSpec

A discriminated union describing how a transition is initiated:

```text
TriggerSpec =
  | { type: "button", id: string }       # named UIButton in the source layer's layout
  | { type: "key",    key: KeyId }       # key event while source is active
  | { type: "event",  name: string }     # programmatic; layer code calls flow.Emit(name)
  | { type: "auto",   afterMs: int }     # fires N ms after the layer enters
```

Button and key are the editor-friendly triggers a designer wires up directly. `event`
is the escape hatch for things only C++ knows about (a match ending, an auth flow
returning). `auto` covers splash → title style flows.

Future enhancement: change `trigger: T` to `triggers: T[]` so one transition can be
bound to both a button click and a key press without duplication.

### Transition kinds

Replace, Push, and Pop share one pipeline; they differ only in the stack-mutation
step.

| kind    | OutAnimating               | Stack mutation                 | InAnimating              |
|---------|----------------------------|--------------------------------|--------------------------|
| Replace | source.TransitionOut       | Remove source, push target     | target.TransitionIn      |
| Push    | (skipped — source stays)   | Push target                    | target.TransitionIn      |
| Pop     | source.TransitionOut       | Remove source                  | (skipped, no target)     |

A `Replace` from inside an overlay back to a root screen (e.g., pause-menu Quit →
Title) is expressed in JSON as a single `kind: replace` on the pause layer. The
runtime is responsible for popping any overlays above the root before performing the
replace; designers don't author intermediate pops.

## Layer-side contract

The framework depends only on a thin contract. Layouts and clips are one
implementation; code-driven layers are another. Both implement the same interface.

```cpp
class ITransitionParticipant {
public:
    virtual void OnEnter() {}
    virtual void OnExit()  {}

    // tag is an opaque string. A clip-driven layer interprets it as a clip
    // name. A code-driven layer interprets it however it likes. The Flow
    // doesn't care; it just calls these and waits for done().
    virtual void TransitionIn (std::string_view tag, std::function<void()> done) = 0;
    virtual void TransitionOut(std::string_view tag, std::function<void()> done) = 0;
};
```

Two concrete bases will ship in moth_ui:

```cpp
// .mothui-backed: tag is a clip name; HasAnimation → SetAnimation → wait for
// EventAnimationStopped → done(). Handles button-binding and key-binding
// installation from the graph automatically in OnEnter via IClickable lookups.
class TransitioningLayer : public moth_ui::Layer,
                            public ITransitionParticipant { ... };

// Code-driven: subclass overrides TransitionIn/Out, drives its own animation,
// calls done() when finished. No layout required.
class CodeDrivenLayer : public moth_ui::Layer,
                        public ITransitionParticipant { ... };
```

The `tag` parameter is the central design point. Because it's just a string, both
implementations participate in exactly the same flow. A `TransitioningLayer`
interprets the tag as a moth_ui clip name and looks it up on its loaded layout. A
`CodeDrivenLayer` switches on the tag string to pick which animation routine to run.
Neither tells the framework anything about its internals.

A layer with no meaningful transition is a degenerate case: it inherits the default
`TransitionIn`/`Out` that immediately fires `done()`. This is the right behaviour for
debug overlays, instant-cut transitions, and toast notifications.

### Layout-backed layer example

```cpp
// Pure pass-through. The layout has buttons "btn_resume" and "btn_quit", and
// the graph's pause-layer JSON has transitions whose triggers reference those
// button ids. TransitioningLayer's OnEnter walks the graph's transitions and
// installs ClickActions on the matching UIButtons automatically. No custom
// logic in this subclass; in fact, no subclass is needed at all if the layer
// is purely graph-driven — see "When to subclass".
class PauseLayer : public moth_ui::flow::TransitioningLayer {
public:
    using TransitioningLayer::TransitioningLayer;
};
```

### Code-driven layer example

```cpp
class TitleSplashLayer : public moth_ui::flow::CodeDrivenLayer {
public:
    void TransitionIn(std::string_view tag, std::function<void()> done) override {
        if (tag == "fast_fade") {
            m_fadeMs = 200;
        } else {
            m_fadeMs = 800;
        }
        m_fadeStart = NowMs();
        m_fadeDone = std::move(done);
    }

    void TransitionOut(std::string_view tag, std::function<void()> done) override {
        // Just snap; this layer doesn't have an exit animation.
        done();
    }

    void Update(uint32_t ticks) override {
        if (m_fadeDone && (NowMs() - m_fadeStart) >= m_fadeMs) {
            auto cb = std::move(m_fadeDone);
            cb();
        }
    }

    void Draw() override {
        float const alpha = ComputeAlpha();
        // Subclass-owned ad-hoc drawing — no .mothui layout, no clips.
        DrawSplashImage(alpha);
    }
};
```

### When to subclass

Most overlays and simple screens shouldn't need a C++ subclass at all — they're a
`.mothui` layout plus a few transitions in the graph. `TransitioningLayer` is the
default factory; the LayerSpec just needs a `layout` path.

A C++ subclass is needed when a layer has state or logic the graph can't express:

- It owns or interacts with a long-lived app object (e.g., `GameLayer` reads from
  `Match`).
- It emits programmatic events back into the flow (`flow.Emit("match_ended")`).
- It draws or animates outside the .mothui layout system (then prefer
  `CodeDrivenLayer`).

In the scorched_moth example, of four current layers, three (`TitleLayer`,
`PauseMenuLayer`, `GameOverLayer`) can become plain JSON entries with no C++
subclass. Only `GameLayer` remains a subclass.

## Buttons and the `IClickable` interface

moth_ui has no native button widget; `nodes/widget.h` is a CRTP extension point for
apps to define their own. The flow system needs to find clickable named nodes on a
layout and attach click actions when a TransitionSpec's trigger is `{type: "button",
id: "..."}`. Two coupled additions to moth_ui solve this:

### `IClickable`

A minimal abstract interface in moth_ui's public surface:

```cpp
namespace moth_ui {
    class IClickable {
    public:
        virtual ~IClickable() = default;
        virtual void SetClickAction(std::function<void()> action) = 0;
    };
}
```

The flow runtime locates buttons via `dynamic_cast<IClickable*>(node)` when walking a
layout. Whatever the widget actually is — moth_ui's bundled `UIButton`, an app's
custom button with hover/sound/animation, an app's image-with-click — it works as
long as it implements `IClickable`. The framework never knows or cares about the
concrete type.

### `UIButton` (moved from scorched_moth)

A `Widget<UIButton>`-based clickable button shipped in moth_ui as an opt-in
convenience. Apps that just want a button drop one into their layout and reference it
from the flow graph. Apps that need richer interaction (hover audio, focus rings,
controller navigation) provide their own widget that implements `IClickable` and
ignore the bundled `UIButton` entirely.

This is the same pattern moth_ui already follows with `Widget<T>` itself — opt-in
primitives that cover the common case without locking out custom implementations.

### Why this matters for the design

`TransitioningLayer::OnEnter` becomes app-subclass-free for trivial screens. It walks
the graph's transitions for this layer, and for each `{type: "button"}` trigger does:

```cpp
if (auto* clickable = dynamic_cast<IClickable*>(m_uiRoot->FindChild(triggerSpec.id).get())) {
    clickable->SetClickAction([this, transitionId]{ m_flow.Trigger(transitionId); });
}
```

A title screen with `Play` and `Quit` buttons is now zero C++ — the layout uses
`UIButton` (or any app widget implementing `IClickable`), the graph JSON wires the
buttons to transitions, and the framework does the rest.

## The Flow runtime

```cpp
namespace moth_ui::flow {

class Flow {
public:
    Flow(LayerStack&, Context&, FlowGraph);

    // Layers whose LayerSpec has factory == null are constructed by the default
    // factory (which builds a TransitioningLayer from layout). Custom factories
    // get registered by name.
    void RegisterFactory(std::string name, LayerFactory);

    // Same registry mechanism for side-effect actions.
    void RegisterAction(std::string name, Action);

    // Creates and pushes the initial layer. Call once, after all factories
    // and actions are registered, before the first Tick.
    void Start();

    // Programmatic trigger of a specific transition by qualified id.
    void Trigger(std::string_view qualifiedId);   // "title.play"

    // Programmatic emission of a named event. The Flow resolves this against
    // the currently-active layer's TriggerSpec list.
    void Emit(std::string_view eventName);

    // Pre-stack event filter — feed every event here before the LayerStack
    // so key triggers get first look. Returns true if consumed.
    bool OnEvent(Event const& event);

    // Pumped from the application's main loop with elapsed milliseconds.
    void Tick(uint32_t ticks);
};

}
```

The `Flow` constructor takes only moth_ui types: a `LayerStack` to mutate, a
`Context` to pass to layer factories, and the loaded `FlowGraph`. It deliberately
does **not** take a `Window` or any other graphics-backend reference — apps that need
backend access in their layer subclasses get it through their own factory closures,
not through the framework. This keeps `Flow` framework-agnostic and embeddable in the
eventual editor process without dragging a window dependency in.

The `Flow` has no knowledge of any specific layer or screen — those are addressed by
string. The application registers the factories and actions that turn strings into
behaviour. The graph references both by name. This is the contract surface between
code and visual editor: when designers ask for new options, the engineer adds a
factory or an action and rebuilds; nothing in the editor changes structurally.

## Transition state machine

Once a trigger fires, the runtime walks a fixed sequence of phases. Each phase can be
synchronous (`done()` fires immediately) or asynchronous (the framework waits for an
explicit callback).

```text
Idle
  └─ Trigger(t) accepted
OnStart        ─── run t.onStart actions sequentially ───────────────┐
  └─ done                                                             │
OutAnimating   ─── source.TransitionOut(outClip, done)               │  each phase can be
  └─ done                                                             │  async; runtime waits
(stack mutation per t.kind)                                           │  for done() before
OnMidpoint     ─── run t.onMidpoint actions sequentially ─────────────│  advancing
  └─ done                                                             │
InAnimating    ─── target.TransitionIn(inClip, done)                  │
  └─ done                                                             │
OnComplete     ─── run t.onComplete actions sequentially ─────────────│
  └─ done                                                             │
Idle ─────────────────────────────────────────────────────────────────┘
```

Stack mutation happens *before* OnMidpoint, not after. This means actions in
OnMidpoint can reach the just-constructed target layer if they want (to inject
pending state into it, for instance). For the common case — `session.NewMatch()`
touching a long-lived `Match` object — the ordering doesn't matter, but the "mutation
first" choice is the more useful default for cases where it does.

For `Push` transitions, `OutAnimating` is skipped. For `Pop`, `InAnimating` is
skipped. OnStart, OnMidpoint, and OnComplete still fire in both cases; OnMidpoint is
the natural place to release per-overlay resources during a pop.

### Re-entry policy

If a `Trigger()` arrives while the machine is non-Idle, the policy field decides:

- **Reject**: log and drop. Current scorched_moth behaviour.
- **Queue** (depth 1): hold the request, apply it when the current transition reaches
  Idle. Default.
- **Coalesce**: last-wins; replace any queued request with the new one.

Queue depth >1 isn't supported in v1; if a designer needs that, they almost certainly
want a different feature (a stack of forward navigations with "go back" semantics,
e.g.).

## Side effects (the action registry)

Side effects are application code triggered from points in the state machine. They
are referenced from JSON by name and registered in C++ by the application.

```cpp
using Action = std::function<void(std::function<void()> done)>;

flow.RegisterAction("session.NewMatch",
    [&session](std::function<void()> done) {
        session.NewMatch(util::Rng::RandomSeed());
        done();
    });

flow.RegisterAction("assets.PreloadGame",
    [&loader](std::function<void()> done) {
        loader.LoadGroup("game", std::move(done));   // genuinely async
    });
```

Actions are always treated as async by the runtime; sync actions just call `done()`
immediately. This keeps one code path and lets long-running side effects (asset
loading, network calls) slot in naturally without special handling.

When multiple actions are listed on the same hook, they run **sequentially in JSON
order**. Designer-authored ordering is preserved, and any one action can take its
time without others jumping ahead.

### Where side effects live in the lifecycle

| Hook        | Fires                                                   | Typical use                                        |
|-------------|---------------------------------------------------------|----------------------------------------------------|
| onStart     | Before source's TransitionOut begins                    | Fade music out; freeze a simulation                |
| onMidpoint  | After stack mutation, before target's TransitionIn      | Reset model state visible during target's fade-in  |
| onComplete  | After target's TransitionIn ends                        | Resume music; start post-entry timers              |

The midpoint slot is the design payoff: state changes that need to be invisible to
the player happen while neither the old nor the new view is animating. The
match-reset example — `session.NewMatch()` between Title → Game — lets the new match
be visible the moment GameLayer's fade-in starts, with no flash of the old state.

### Parameters — punted

Actions are parameterless in v1. `session.NewMatch` reads its seed from app-side
state (a "pending session config" or similar set up before triggering). Adding
parameter passing through the JSON would require either:

1. Per-action parameter schemas (so the editor can render UI for them).
2. Opaque JSON blobs (no editor support).
3. Stringly-typed action factories (`"session.NewMatch[seed=random]"`).

None of these is right for v1. The editor isn't here yet, and app-side state is
sufficient. Revisit once the editor lands: option 1 with per-action schemas is the
right long-term answer but is a meaningful authoring-UI investment.

### Error handling and timeouts

If an action's `done()` is never called, the transition stalls. The framework
enforces a per-action timeout (configured in `GraphPolicy.actionTimeoutMs`) and
force-advances on expiry with a loud log. The same policy applies to layers'
`TransitionIn`/`Out` callbacks. Missing callbacks are almost always bugs and the
runtime must not silently swallow them.

If an action throws, the framework logs and force-advances. Transitions in flight do
not get rolled back; the system is forward-only.

### Cancellation — deliberately not supported

Once a transition starts, it commits. "Conditional navigation" (e.g.,
confirm-before-quit) is the app's responsibility: the title's quit button shows a
confirmation overlay, and only that overlay's OK button calls
`Trigger("title.quit")`. Adding conditional cancellation to the graph would turn the
runtime into a workflow engine, which is a different and much bigger product.

Similarly, actions don't return values back into the graph. If the C++ side needs to
decide between transitions based on some outcome, it decides in code and triggers a
different transition.

## Modality

Overlays carry a `modality` field that controls how the layers below them behave
while the overlay is up:

- `Modal`: `moth_ui::LayerStack` short-circuits event delivery and `Update` calls to
  layers beneath this one. The pause menu doesn't need a manual `m_paused` flag in
  `GameLayer` — being covered by a modal overlay is what pauses the game.
- `Passthrough`: layers beneath continue to update and receive non-handled input.
  Useful for HUD overlays, debug panels.

The `m_actionTriggered` double-click guard recurring in scorched_moth's overlay
subclasses is subsumed by the framework's "transition already in progress" guard:
once a button click triggers a transition, the runtime is non-Idle, and further
clicks are rejected (or queued, per policy).

Implementation note: modal-skip is a `moth_ui::LayerStack` semantic extension, not
purely a flow-system addition. The flag the flow system reads (`LayerSpec.modality`)
maps to a property the LayerStack honours during its update/event-dispatch loop. The
LayerStack change is the foundational dependency that lands before the flow system
itself.

## Consequences for application code

The scorched_moth example, projected against the new system:

- `Flow` class in `src/flow/` disappears in favour of `moth_ui::flow::Flow`
  constructed from a JSON-loaded `FlowGraph`.
- `enum class Screen` disappears; layers are identified by string id in the graph.
- `IFlowLayer` disappears (subsumed by `ITransitionParticipant`).
- `UILayer` disappears (subsumed by `TransitioningLayer`).
- `UIButton` moves to moth_ui as an opt-in widget; scorched_moth includes the moth_ui
  header instead of carrying its own copy.
- `TitleLayer`, `PauseMenuLayer`, `GameOverLayer` disappear as C++ classes; they
  become layer entries in the graph JSON.
- `GameLayer` remains as a C++ subclass because it owns and renders the game world.
  It becomes a view-only object: `Match` is owned externally (by an
  `Application`-level `Session` object or similar), and `GameLayer` reads from it.
  Replays don't reconstruct `GameLayer` — they call `Session::NewMatch()` from an
  `onMidpoint` action on the relevant transition and let the existing `GameLayer`
  re-enter via `OnEnter`.
- `m_actionTriggered` guards, manual `PushOverlay`/`PopOverlay` calls, lambda chains
  threaded through constructors, and the per-layer `m_paused` flag all disappear.

The remaining C++ surface for the app is: register factories, register actions, load
the graph, pump `Tick`. Everything else is data.

## Public API additions to moth_ui

This design introduces several new public headers in moth_ui. They're all opt-in —
including none of them leaves existing moth_ui users unaffected.

- `moth_ui/flow/flow.h` — the `Flow` runtime.
- `moth_ui/flow/flow_graph.h` — `FlowGraph`, `LayerSpec`, `TransitionSpec`,
  `TriggerSpec` and JSON loaders.
- `moth_ui/flow/transition_participant.h` — the `ITransitionParticipant` interface.
- `moth_ui/flow/transitioning_layer.h` — the `.mothui`-backed layer base.
- `moth_ui/flow/code_driven_layer.h` — the code-driven layer base.
- `moth_ui/flow/iclickable.h` — the `IClickable` interface. Lives under `flow/`
  because the flow runtime is its only current consumer; non-flow consumers can
  include it directly if they grow a need for it.
- `moth_ui/widgets/ui_button.h` — the bundled `UIButton` widget that implements
  `IClickable`. (Lives under `widgets/` alongside `widget.h`, the CRTP base.)

The `moth_ui/flow/` subdirectory is a coherent unit; users can ignore it wholesale or
pull in the pieces they want.

## Editor implications (forward-looking)

- Each LayerSpec is a card in a node diagram.
- Each TransitionSpec is an arrow between two cards.
- Each arrow exposes: a target dropdown (other layers); a kind dropdown; an out-clip
  dropdown (clips on source's layout); an in-clip dropdown (clips on target's
  layout); a trigger configurator; three lists (onStart/onMidpoint/onComplete) of
  action names from a dropdown populated by the app's registered actions.
- The action registry is the contract surface between code and editor. Adding new
  options means adding actions in C++ and rebuilding; the editor's UI is data-driven
  from the registry.
- Button-trigger dropdowns enumerate layout node ids whose factory-registered type
  implements `IClickable`. The editor learns about clickable widget types via the
  same `NodeFactory` registration `Widget<T>` uses today.
- For layout-backed layers the editor can instantiate the actual `TransitioningLayer`
  and play transitions live in-tool. Code-driven layers can't preview at design time;
  the editor should show a stubbed placeholder and be honest about the limitation.

To keep that editor possible, the runtime needs to be embeddable in the editor
process. The flow system's moth_ui-only dependency footprint already supports this:
the editor pulls in moth_ui (it does today) and gets the flow runtime for free, with
no moth_graphics window/graphics dragged in. Two further things to watch for during
implementation: avoid `main()`-level singletons (the current scorched_moth code
already avoids this), and don't bake global logger configuration into the Flow
itself.

## Validation surface (at graph-load time)

A graph integrity check (`ValidateFlowGraph`) runs once and reports all
failures at once rather than stopping on the first. The JSON loader calls it
automatically; a **hand-built graph must call it explicitly** (scorched_moth
does, in `PostCreateWindow`).

**Implemented today** — purely structural checks that need no registries or
layouts:

- Layer ids are unique.
- The `initial` layer exists.
- Every `to` field references a defined layer (or `"<back>"`).
- No `Pop` transition has a non-`"<back>"` target, and no non-`Pop`
  transition targets `"<back>"`.
- No `Push` transition targets a `Screen`-kind layer (overlays can only push
  overlays).

**Designed but NOT yet implemented** — the checks below were intended but
the current validator does not perform them. These references resolve
*lazily* and fail (or no-op) at trigger time, not at load:

- `outClip` / `inClip` reference a real clip on the relevant layout.
- `trigger.button.id` references a node present on the layout whose type
  implements `IClickable`.
- `factory` references a registered factory.
- Every `ActionRef` references a registered action.

Until those land, treat clip names, button ids, factory names, and action
names as unchecked strings: keep them under test coverage or grep them
against your layouts and registrations when wiring transitions.

## Open questions deferred

- **Wildcard transitions (`from: "*"`).** Designers will eventually want "Esc from
  any screen → settings overlay." Authoring sugar in the editor can expand to
  per-layer entries on save in v1; revisit a true wildcard in the runtime if the
  duplication becomes painful.
- **Multi-trigger transitions.** `triggers: T[]` instead of `trigger: T`.
  Mechanically easy; defer until the editor exists and the authoring pain is
  observable.
- **Action parameters.** Per-action schemas plus editor UI; revisit alongside the
  editor.
- **Cached layer construction lifecycle.** When a `construction: Cached` layer is
  evicted (memory pressure, debug action), what happens? v1 keeps cached layers
  forever; revisit if memory matters.
- **Auto-trigger pause semantics.** A layer with an `auto` trigger has its timer
  ticking while it's the topmost. What happens when a modal overlay covers it?
  Default: pause the timer (consistent with "modal blocks updates below"). Document
  but verify in implementation.
- **History / back-stack.** No first-class history beyond the overlay stack. If apps
  need a "go back" stack across screen replaces, that's a future feature, not v1.

## Out of scope for the first cut

- Wildcard transitions.
- Multi-trigger transitions.
- Action parameters.
- Auto triggers with `afterMs` countdown UI.
- The visual editor itself.
- History / back-stack across screens.
- Cancellation, conditional flow, return values from actions.
- Cross-process / hot-reload of graphs.

Everything not on this list is v1.
