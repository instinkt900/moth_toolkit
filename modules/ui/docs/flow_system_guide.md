# Flow System — Consumer Guide

How to use the moth_ui flow system to drive screen/overlay navigation in
your application. This is the practical how-to; for the design rationale
(why transitions nest under layers, why the stack mutates before the
midpoint, deferred editor scope) see
[`flow_system_design.md`](flow_system_design.md).

The worked example throughout is **scorched_moth**, the framework's main
consumer. Its graph is assembled in `src/scorched_application.cpp`
(`BuildFlowGraph()` plus the factory/action registrations); its layers
live in `scorched_moth/src/layers/`.

## What it is

A navigation runtime. You describe your app as a graph of **layers**
(screens and overlays) and the **transitions** between them. The runtime
plays the in/out animations, mutates the layer stack, and sequences your
side effects, so layer code stays focused on its own job instead of
push/pop plumbing and double-click guards.

It's pure moth_ui — no moth_graphics dependency. Existing moth_ui apps
that roll their own layer management can ignore the `moth_ui/flow/`
headers entirely and lose nothing.

## The two halves

- **`FlowGraph`** — plain data: the layers, their transitions, triggers,
  and side-effect hooks. Either hand-built in C++ or loaded from JSON.
- **`Flow`** — the runtime. You hand it the graph plus references to your
  `LayerStack` and `Context`, register the factories and actions the graph
  names by string, and pump it each frame.

Everything the graph references — layer factories, side-effect actions —
is resolved by **name** against registries you populate in C++. The Flow
itself knows nothing about any specific screen.

## Wiring the runtime into your app

Four steps at startup, then two calls per frame. From scorched_moth:

```cpp
// 1. Build (or load) the graph, then validate it. Hand-built graphs are
//    NOT validated for you — call the validator yourself or structural
//    errors only surface lazily at Trigger time.
auto graph = BuildFlowGraph();
auto errors = moth_ui::flow::ValidateFlowGraph(graph);
for (auto const& e : errors) {
    spdlog::error("flow graph error [{}.{}]: {}",
                  e.layerId, e.transitionId, e.message);
}
// (bail if !errors.empty())

// 2. Construct the runtime. Takes only moth_ui types — no Window.
m_flow = std::make_unique<moth_ui::flow::Flow>(
    m_window->GetLayerStack(), m_window->GetMothContext(), std::move(graph));

// 3. Register custom layer factories (by name) for layers that need a
//    C++ subclass. Closures capture whatever app state the layer needs.
m_flow->RegisterFactory("game",
    [this](moth_ui::Context& ctx, moth_ui::flow::LayerSpec const&) {
        return std::make_unique<RoundLayer>(
            ctx, *m_flow, *m_session, m_window->GetGraphics());
    });

// 4. Register side-effect actions (by name).
m_flow->RegisterAction("session.StartMatch",
    [this](std::function<void()> done) { m_session->StartMatch(); done(); });

// Create + push the initial layer. Call once, after registrations.
m_flow->Start();
```

Per frame:

```cpp
bool OnEvent(moth_ui::Event const& event) override {
    // Feed the Flow BEFORE the LayerStack so key triggers get first look.
    if (m_flow && m_flow->OnEvent(event)) {
        return true;   // consumed by a key trigger
    }
    return Application::OnEvent(event);
}

void TickFixed(uint32_t ticks) override {
    if (m_flow) {
        m_flow->Tick(ticks);   // advances the transition state machine
    }
}
```

That's the entire required surface: **construct, register, `Start`, then
`OnEvent` + `Tick`**.

> **Self-registered widgets.** If your buttons (or other widgets) register
> themselves with the `NodeFactory` and are referenced only by layout-class
> string, the linker may strip them. Call `moth_ui::EnsureWidgetsRegistered()`
> once at startup (scorched_moth does this in `PostCreateWindow`) so the
> bundled widgets survive.

## Building the graph

### Hand-built (what scorched_moth does)

The graph is just structs. scorched_moth assembles it in code rather than
loading JSON — equivalent to what a designer-authored `flow.json` would
describe, but driven without the loader:

```cpp
moth_ui::flow::FlowGraph graph;
graph.initial = "title";
graph.policy.onReentry = moth_ui::flow::ReentryPolicy::Queue;

moth_ui::flow::LayerSpec title;
title.id = "title";
title.layout = "assets/layouts/screen_title.mothui";
title.kind = moth_ui::flow::LayerKind::Screen;

moth_ui::flow::TransitionSpec play;
play.id = "play";
play.to = "match_setup";
play.kind = moth_ui::flow::TransitionKind::Replace;
play.inClip = "transition_in";
play.trigger.kind = moth_ui::flow::TriggerKind::Button;
play.trigger.id = "btn_start";
title.transitions.push_back(std::move(play));

graph.layers.push_back(std::move(title));
```

### JSON-loaded

`LoadFlowGraphFromFile(path)` / `LoadFlowGraphFromJson(json)` return a
`LoadResult { std::optional<FlowGraph> graph; std::vector<…> errors; }`
with `ok()` true when parse + structural validation both pass. The JSON
loader runs `ValidateFlowGraph` for you; the hand-built path does not.

The same structs map straight to JSON. Enum fields are strings, matched
case-insensitively, and every field with a default may be omitted. This
graph — a title screen, a game screen, and a modal pause overlay — is
equivalent to the hand-built form above:

```json
{
  "initial": "title",
  "policy": { "onReentry": "queue", "actionTimeoutMs": 5000 },
  "layers": [
    {
      "id": "title",
      "layout": "assets/layouts/screen_title.mothui",
      "kind": "screen",
      "transitions": [
        {
          "id": "play",
          "to": "game",
          "kind": "replace",
          "inClip": "transition_in",
          "trigger": { "type": "button", "id": "btn_start" },
          "onMidpoint": ["session.StartMatch"]
        },
        {
          "id": "quit",
          "to": "title",
          "kind": "replace",
          "trigger": { "type": "button", "id": "btn_quit" },
          "onStart": ["app.Quit"]
        }
      ]
    },
    {
      "id": "game",
      "factory": "game",
      "layout": "assets/layouts/game_hud.mothui",
      "kind": "screen",
      "transitions": [
        {
          "id": "pause",
          "to": "pause",
          "kind": "push",
          "trigger": { "type": "key", "key": "Escape" }
        }
      ]
    },
    {
      "id": "pause",
      "layout": "assets/layouts/screen_pause_menu.mothui",
      "kind": "overlay",
      "modality": "modal",
      "transitions": [
        {
          "id": "resume",
          "to": "<back>",
          "kind": "pop",
          "trigger": { "type": "button", "id": "btn_resume" }
        }
      ]
    }
  ]
}
```

Field notes: `kind` defaults to `"replace"` (transitions) / `"screen"`
(layers); a `Key` trigger names the key by its `moth_ui::Key` enum name
(e.g. `"Escape"`); an `Event` trigger takes `"id"` (or `"name"`); an
`Auto` trigger takes `"afterMs"`. JSON moves only the graph *structure*
into data — the `factory` (`"game"`) and the action names
(`"session.StartMatch"`, `"app.Quit"`) still have to be registered in C++
exactly as with a hand-built graph.

### LayerSpec fields

| Field             | Meaning                                                      |
|-------------------|-------------------------------------------------------------|
| `id`              | Unique layer id, e.g. `"title"`, `"game"`, `"pause"`.       |
| `factory`         | `std::optional` name of a registered factory; empty → default (a `TransitioningLayer` from `layout`). |
| `layout`          | Path to a `.mothui`, consumed by the default factory.       |
| `kind`            | `LayerKind::Screen` or `Overlay`.                           |
| `modality`        | `Modality::Modal` (default) or `Passthrough` — overlays only. |
| `construction`    | `Construction::Fresh` (default, rebuilt each entry) or `Cached` (instance preserved across visits). |
| `defaultInClip`   | Fallback in-clip name (default `"transition_in"`).          |
| `defaultOutClip`  | Fallback out-clip name (default `"transition_out"`).        |
| `transitions`     | Outgoing edges from this layer.                             |

### TransitionSpec fields

| Field        | Meaning                                                           |
|--------------|------------------------------------------------------------------|
| `id`         | Local to the parent layer; addressed globally as `"layerId.transitionId"`. |
| `to`         | Target layer id, or `"<back>"` (Pop only — pops the topmost overlay). |
| `kind`       | `Replace`, `Push`, or `Pop`.                                     |
| `inClip`     | `std::optional` clip on the **target**'s layout; empty → target's `defaultInClip`. |
| `outClip`    | `std::optional` clip on the **source**'s layout; empty → source's `defaultOutClip`. |
| `trigger`    | How the transition fires (below).                                |
| `onStart` / `onMidpoint` / `onComplete` | Lists of registered action names, run at fixed points (below). |

## Triggers

A `TriggerSpec` has a `kind` and (for Button/Event) an `id`:

- **`Button`** — `id` is the layout node id of an `IClickable` widget on the
  source layer. The runtime auto-binds the click for you (see below).
- **`Key`** — `trigger.key` is a `moth_ui::Key`; fires on key-down while the
  source layer is the topmost active layer. This is why `Flow::OnEvent`
  must see events before the LayerStack.
- **`Event`** — `id` is a name your layer code passes to
  `m_flow.Emit("name")`. The escape hatch for things only C++ knows (a
  match ending, an async callback returning). scorched_moth's `RoundLayer`
  emits `"match_ended"` when the round ends.
- **`Auto`** — `trigger.afterMs` fires the transition N ms after the layer
  becomes active (splash → title style).

A transition with **no trigger set** is never fired by user input — you
fire it yourself with `m_flow.Trigger("layerId.transitionId")`. scorched_moth
uses these for programmatic hops like `game.open_shop` and the
disconnect-notice transitions.

## Layers

### The default: `TransitioningLayer`

A layer whose spec has no `factory` is built as a `TransitioningLayer` from
its `layout`. It loads the `.mothui`, dispatches events into it, and
interprets each transition `tag` as a **clip name**: if the clip exists it
plays it and waits for `EventAnimationStopped` before calling `done()`;
otherwise `done()` fires immediately. A purely graph-driven screen (title,
pause menu) needs **no C++ subclass at all**.

Button auto-binding: when a layer mounts, the Flow walks its transitions
and, for each `Button` trigger, does
`dynamic_cast<IClickable*>(root->FindChild(id).get())->SetClickAction(...)`.
The widget can be the bundled `moth_ui::UIButton` or any app widget that
implements `IClickable` — the runtime only knows the interface.

### When to subclass

Subclass `TransitioningLayer` when a layer holds state or behaviour the
graph can't express:

- it reads/owns a long-lived app object (scorched_moth's `RoundLayer`
  reads the `Session`/`Round`);
- it emits programmatic events back into the flow (`m_flow.Emit(...)`);
- it intercepts a flow-wired button to add gating logic (see the gotcha
  below).

Register the subclass with `RegisterFactory(name, …)` and set the spec's
`factory` to that name.

### `CodeDrivenLayer`

For layers with no `.mothui` at all — you override `TransitionIn`/`Out`,
drive your own animation, and call `done()` when finished. The `tag` is
yours to interpret however you like. Use this for ad-hoc drawn screens.

## Transition kinds, the phase machine, and side effects

Replace, Push, and Pop share one pipeline and differ only in the
stack-mutation step:

| kind    | OutAnimating         | Stack mutation             | InAnimating         |
|---------|----------------------|----------------------------|---------------------|
| Replace | source.TransitionOut | remove source, push target | target.TransitionIn |
| Push    | *(skipped)*          | push target                | target.TransitionIn |
| Pop     | source.TransitionOut | remove source              | *(skipped)*         |

Every trigger walks this sequence; each phase may be sync or async (the
runtime waits for `done()` before advancing):

```
Idle → onStart → OutAnimating → [stack mutation] → onMidpoint
     → InAnimating → onComplete → Idle
```

Side-effect hooks (lists of action names, run **sequentially in order**):

| Hook         | Fires                                              | Typical use                          |
|--------------|----------------------------------------------------|--------------------------------------|
| `onStart`    | before the source's out-animation                  | fade music out; freeze a sim         |
| `onMidpoint` | after stack mutation, before the target's in-anim  | inject state into the just-built target; reset model invisibly |
| `onComplete` | after the target's in-animation                    | resume music; start post-entry timers |

The stack mutates **before** `onMidpoint`, so midpoint actions can reach
the freshly-constructed target. scorched_moth uses `onMidpoint` for
`session.StartMatch` / `session.NextRound` so the new round is live the
instant the game layer fades in.

Actions are `std::function<void(std::function<void()> done)>` and are
always treated as async — synchronous work just calls `done()` immediately.
A long-running action (asset load, network round-trip) slots in with no
special handling, but **must call `done()` exactly once** or the transition
stalls (the watchdog below will eventually force it).

## Overlays, modality, and `<back>`

- **Screen** layers replace the base; only one is the base at a time.
- **Overlay** layers (`Push`) stack on top. `Pop` with `to = "<back>"`
  removes the topmost overlay.
- **Modality** controls layers *below* an overlay: `Modal` (default) stops
  the LayerStack from delivering events/updates beneath it — being covered
  by a modal pause menu is what pauses the game, no `m_paused` flag needed.
  `Passthrough` lets lower layers keep updating (HUDs, debug panels).

A `Replace` from inside an overlay back to a base screen pops intervening
overlays for you. But the runtime is deliberately minimal: for **multi-step
hops** (pop an overlay, then replace the screen underneath) compose them in
your own code with a `Pop` whose `onComplete` action calls
`m_flow.Trigger(...)` for the next leg — don't expect a single transition to
do both. scorched_moth's results-overlay quit does exactly this
(`round_results.quit` → `app.QuitToTitle` → `game.quit`).

## Re-entry policy and the timeout watchdog

If a `Trigger`/`Emit` arrives mid-transition, `GraphPolicy.onReentry`
decides: **Reject** (drop + log), **Queue** (hold one, apply at Idle —
scorched_moth's choice), or **Coalesce** (last-wins). This subsumes the
old per-overlay double-click guards: once a click starts a transition the
runtime is non-Idle and further clicks are dropped or queued.

A per-phase watchdog guards async work: if an animation or action doesn't
call `done()` within `GraphPolicy.actionTimeoutMs` (default 5000 ms) the
runtime logs a warning and force-advances. Missing callbacks are bugs; the
runtime won't silently hang on them.

## Validation — and what it does *not* catch

`ValidateFlowGraph(graph)` returns a list of `GraphValidationError`. The
JSON loader runs it automatically; **a hand-built graph does not — call it
yourself.** It currently checks:

- layer-id uniqueness;
- `initial` references a real layer;
- every transition `to` references a defined layer (or `"<back>"`);
- `Pop` transitions target `"<back>"`, and non-`Pop` transitions don't;
- `Push` targets an `Overlay`, not a `Screen`.

It does **not** (yet) verify clip names, button-node ids, factory names, or
action names. Those resolve lazily: a typo'd `inClip`, a `trigger.id` that
doesn't match any node, an unregistered `factory` or action name fails (or
no-ops) at trigger time, not at load. Keep those strings under test
coverage, or grep them against your layouts/registrations when wiring new
transitions.

## Gotchas

- **Button bindings install before `OnEnter`.** The Flow wires flow-graph
  button clicks when the layer mounts, *before* your `OnEnter` runs. A
  layer constructor therefore can't intercept a flow-wired button — if you
  need to override a button's action (e.g. to gate on networked readiness),
  re-set it from `OnEnter`, not the ctor. scorched_moth's `MatchSetupLayer`
  and `RoundResultsLayer` do this for `btn_prepare` / `btn_gonext`.
- **`inClip` lives on the target, `outClip` on the source.** A transition
  names an in-clip defined on the layer it's heading *to*.
- **No history/back-stack across screens.** `"<back>"` only pops the
  overlay stack; there's no automatic "previous screen" memory.
- **Cached layers live forever (v1).** A `Construction::Cached` layer's
  instance is reused across visits and never evicted.
- **Keep the runtime narrow.** Niche multi-layer transition choreography
  belongs in app code (chained `Pop` + `onComplete` + `Trigger`), not in
  new runtime features.

## Header map

All opt-in; including none leaves existing moth_ui users unaffected.

| Header                                  | Provides                                  |
|-----------------------------------------|-------------------------------------------|
| `moth_ui/flow/flow.h`                   | the `Flow` runtime                        |
| `moth_ui/flow/flow_graph.h`             | `FlowGraph`, `LayerSpec`, `TransitionSpec`, `TriggerSpec`, the enums, the validator, and JSON loaders |
| `moth_ui/flow/transition_participant.h` | the `ITransitionParticipant` interface    |
| `moth_ui/flow/transitioning_layer.h`    | the `.mothui`-backed layer base           |
| `moth_ui/flow/code_driven_layer.h`      | the code-driven layer base                |
| `moth_ui/flow/iclickable.h`             | the `IClickable` interface                |
| `moth_ui/widgets/ui_button.h`           | the bundled `UIButton` widget             |
