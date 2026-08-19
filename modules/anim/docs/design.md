# Character Animation — Design

Status: implemented. `gfx::Sprite` now carries flip, playback speed, and
clip start/stop/loop callbacks (see
`modules/gfx/include/moth/graphics/graphics/sprite.h`), and the `moth::anim`
module (state machine + JSON loader + factory) lives under `modules/anim/`. This
document remains the design-of-record; where it and the headers disagree, the
headers win.

The design also proposes a small set of generic, reusable additions to
`moth::gfx::Sprite` (flip, playback speed, lifecycle callbacks). Those additions
live in `moth_gfx` because they are useful to any game sprite, not just characters;
`moth::anim` builds on top of them.

Location: a new `moth_anim` Conan package / `moth::anim` CMake target, depending
on `moth_gfx` and `moth_core`. It is a compiled library (state-machine and
JSON-parsing logic in `.cpp` files), unlike the header-only `moth_ecs`.

## Goal

Give game code a declarative way to drive a character's sprite-sheet animations.
A character has named *states* (`idle`, `walk`, `run`, `jump`, `landing`, …), each
mapping to a flipbook clip. Game code calls `animator.TransitionTo("run")` and the
module plays the correct clip — including a one-shot *transition clip* between
states when one is authored — and reports lifecycle events (state entered/exited,
clip started/stopped) that the game can hook for sound, VFX, or gameplay.

The long-term goal is the same as `moth::ui::flow`: keep the data declarative,
serialisable, and statically validatable so a designer-facing tool can author
character animation sets without writing C++.

## What already exists (building blocks)

- `gfx::SpriteSheet` (`modules/gfx/include/moth/graphics/graphics/spritesheet.h`)
  holds an atlas `Image`, per-frame rects/pivots, and named `ClipDesc`s. A clip is
  an ordered `ClipFrame{ frameIndex, durationMs }` sequence plus a `LoopType`
  (`Stop | Reset | Loop`). This is the data model we reuse unchanged.
- `gfx::Sprite` (`…/sprite.h`) is the playback cursor: `SetClip`, `SetPlaying`,
  `Update(ms)`, `SetFrame`, and `GetCurrentFrameRect/Pivot/…`. It implements the
  loop behaviour but currently exposes no flip, no speed control, and no events.
- `gfx::SpriteSheetFactory` (`…/spritesheet_factory.h`) loads `.flipbook.json`
  descriptors (produced by `moth_packer`), cached by path.
- `gfx::SpriteBatch` (`…/sprite_batch.h`) batches and z-sorts sprites and already
  carries `flipX`/`flipY`, but it is a draw-time helper, not an animation player.

## Gap analysis

| Need | Status today | Plan |
|------|--------------|------|
| Named clips + per-frame timing | `gfx::Sprite` | reuse as-is |
| Horizontal flip for facing left/right | only `IGraphics::DrawImage` Transform2D overload and `SpriteBatch`; `gfx::Sprite`/`DrawSprite` have none | add `SetFlipX`/`GetFlipX` to `gfx::Sprite`; `DrawSprite` honours it |
| Clip started/stopped/looped callbacks | `moth::ui::NodeFlipbook` fires `EventFlipbookStarted`/`EventFlipbookStopped`, but that is UI-node specific; `gfx::Sprite` has none | add `OnClipStarted`/`OnClipStopped`/`OnClipLooped` to `gfx::Sprite` |
| Playback speed | durations baked into `.flipbook.json`; no runtime control | add `SetSpeed`/`GetSpeed` to `gfx::Sprite` |
| Per-state → clip mapping | none | `Animator` + `AnimSet` in `moth::anim` |
| Transition clips between states | none | `TransitionSpec.clip` in `moth::anim` |
| One-shot completion ("jump ends → landing") | `gfx::Sprite` just stops; caller must poll `IsPlaying()` | `onComplete` in `StateSpec`, built on `Sprite::OnClipStopped` |

Two design consequences:

1. **`moth::gfx` gains three generic, minimal `Sprite` features** — flip, speed,
   and lifecycle callbacks. Each is independently useful without `moth::anim`
   (a bullet or pickup uses flip and a stopped callback with no state machine).
   The additions are deliberately narrow: they do not introduce an event system,
   an ECS, or any UI dependency into `gfx`.
2. **`moth::ui`'s flipbook is not reused and not replaced.** The UI flipbook is a
   layout node driven by keyframe tracks; `moth::anim` targets game characters.
   The two coexist and share only the underlying `.flipbook.json` clip model.

## gfx::Sprite extensions (proposed)

The base sprite is the single place that owns flip, timing, and clip lifecycle.
Because timing stays in `gfx::Sprite`, clip events — including the per-loop event —
are native rather than inferred by a wrapper.

```cpp
// Proposed additions to moth::gfx::Sprite (moth/graphics/graphics/sprite.h).

// Render-time mirroring. Independent of playback; a flip does not reset the
// clip or its position. DrawSprite (all overloads) honours it by routing
// through the Transform2D overload of IGraphics::DrawImage.
void  SetFlipX(bool flipX);
bool  GetFlipX() const;

// Playback multiplier applied to Update(ticks). 1.0f is normal. Only speeds
// > 0 are accepted; any speed <= 0 logs a warning and resets to 1.0f.
// (Reverse playback is deferred — see "Resolved decisions".)
void  SetSpeed(float speed);
float GetSpeed() const;

// Clip lifecycle callbacks. Default-empty; assigned by the caller. Fired from
// Update(), never re-entrantly from SetClip/SetPlaying.
std::function<void(std::string_view clipName)> OnClipStarted;  // clip begins advancing
std::function<void(std::string_view clipName)> OnClipStopped;  // Stop/Reset clip reaches its end
std::function<void(std::string_view clipName)> OnClipLooped;   // Loop clip wraps to the first step
```

Semantics to pin down during implementation:

- `OnClipStarted` fires on the first `Update` after playback actually begins
  (a clip set while paused fires it when `SetPlaying(true)` next advances),
  matching the deferred-started pattern in `moth::ui::NodeFlipbook`.
- `OnClipStopped` fires for both `LoopType::Stop` and `LoopType::Reset`, matching
  `moth::ui::EventFlipbookStopped`. It never fires for `Loop`.
- `OnClipLooped` fires each time a `Loop` clip wraps to its first step.
- `SetSpeed` rejects non-positive values: it logs a warning and resets to `1.0f`.
  Pausing is expressed with `SetPlaying(false)`, never with speed.
- `DrawSprite` currently maps rects through the rect-based `DrawImage`, which
  does not flip. The overloads are updated to use the Transform2D overload so
  `flipX` is honoured, with the pivot taken from the frame's `FrameEntry.pivot`.

No change to `SpriteSheet` or the `.flipbook.json` format is required.

## Core model (moth::anim)

### AnimSet

The serialisable artifact. Describes states and the transitions between them. It
does **not** describe frames or clips — those live in the flipbook the `AnimSet`
references. Separation of concerns: flipbook = frames + clips, animset = states +
transitions.

```text
AnimSet {
    initial:     StateId
    flipbook:    string        # path to a .flipbook.json, loaded via SpriteSheetFactory
    states:      StateSpec[]
    transitions: TransitionSpec[]   # only the *exceptional* edges; see below
}
```

### StateSpec

One node. A state is primarily "play this clip".

```text
StateSpec {
    id:          StateId          # unique within the set, e.g. "idle", "run"
    clip:        string           # clip name in the flipbook
    onComplete:  StateId | null   # where to go when a non-looping clip finishes
}
```

`onComplete` is only meaningful for clips whose `LoopType` is `Stop` or `Reset`.
It is what turns `jump` → `landing` → `idle` into data instead of polling code.
If `onComplete` is null, a non-looping clip freezes on its last frame and playback
stops. A looping clip ignores `onComplete`. Both `Stop` and `Reset` fire
`onComplete` identically; the only difference is which frame is left visible
(`Stop` holds the last step, `Reset` rewinds to the first).

### TransitionSpec

One exceptional edge. The common case — "any state to any other state is an
instant cut to the target's clip" — is implicit and needs no entry. A
`TransitionSpec` exists only to attach a transition clip to a specific edge.

```text
TransitionSpec {
    id:    string              # readable as "idle.run"
    from:  StateId
    to:    StateId
    clip:  string | null       # one-shot clip played before the target clip; null = instant
}
```

Why edges are implicit by default: a character with N states has up to N² edges,
and authoring all of them by hand is not viable. Games almost always want
"switch to the target clip immediately" except for a handful of states that need a
transition animation (e.g. `idle → run` plays `run_start`, or `run → jump` plays
`jump_takeoff`). Making the instant cut the default and `transitions` the override
keeps the JSON small and the runtime simple.

### Example descriptor

`player.anim.json`:

```json
{
  "initial": "idle",
  "flipbook": "player.flipbook.json",
  "states": [
    { "id": "idle",    "clip": "idle" },
    { "id": "walk",    "clip": "walk" },
    { "id": "run",     "clip": "run" },
    { "id": "jump",    "clip": "jump",     "onComplete": "landing" },
    { "id": "landing", "clip": "landing",  "onComplete": "idle" }
  ],
  "transitions": [
    { "id": "idle.run", "from": "idle", "to": "run", "clip": "run_start" },
    { "id": "walk.run", "from": "walk", "to": "run", "clip": null }
  ]
}
```

The `clip: null` edge on `walk.run` is a no-op (it equals the implicit default) and
would only exist as an explicit "I considered this edge and want a cut" statement;
it can be omitted.

## Animator (the state machine)

The `Animator` owns an `AnimSet` and a `gfx::Sprite`, and drives the latter. Its
public contract is tiny and state is addressed by string, mirroring
`moth::ui::flow::Flow`. It owns the sprite's callback slots for its internal
sequencing and re-exposes clip-level callbacks of its own; when a sprite is driven
through an `Animator`, game code hooks the `Animator`'s callbacks rather than
assigning the sprite's directly.

```cpp
namespace moth::anim {

class Animator {
public:
    Animator(std::shared_ptr<gfx::SpriteSheet> sheet, AnimSet set);

    // Switch to a state, resolving any authored transition clip.
    void TransitionTo(std::string_view stateId);

    std::string_view GetCurrentState() const;
    void Update(uint32_t ms);

    // The underlying sprite: flip/speed/render surface all live here.
    gfx::Sprite& GetSprite();

    // State-level lifecycle callbacks.
    std::function<void(std::string_view stateId)> OnStateEntered;
    std::function<void(std::string_view stateId)> OnStateExited;
    std::function<void(std::string_view transitionId)> OnTransitionStarted;
    std::function<void(std::string_view transitionId)> OnTransitionCompleted;

    // Clip-level callbacks (forwarded from the underlying sprite).
    std::function<void(std::string_view clipName)> OnClipStarted;
    std::function<void(std::string_view clipName)> OnClipStopped;
    std::function<void(std::string_view clipName)> OnClipLooped;
};

}
```

### Transition pipeline

`TransitionTo(to)` from a current state `from`:

1. Resolve an edge. If a `TransitionSpec` for `from → to` exists, it may carry a
   transition clip; otherwise the transition is an instant cut.
2. If a transition clip exists, play it once (an ephemeral clip, not a state).
3. When the transition clip completes (or immediately, for a cut), enter `to`:
   fire `OnStateExited(from)`, `OnStateEntered(to)`, and play `to.clip`.

```text
TransitionTo(to)
  ├─ edge (from→to) has clip ──> play transition clip (once)
  │       └─ OnClipStopped ──────┐
  ├─ edge is a cut ──────────────┤
  └──────────────────────────────┘
                              fire OnStateExited(from), OnStateEntered(to)
                              play to.clip  (loop, or one-shot → onComplete)
```

The transition clip is driven through the same `gfx::Sprite`, so `Animator`
subscribes to the sprite's `OnClipStopped` to know when the ephemeral clip ends and
the target clip should begin.

### One-shot state completion

When a state's clip is non-looping and finishes, `onComplete` decides the next
state. This is how `jump → landing → idle` runs without game code: the game calls
`TransitionTo("jump")` once on the jump input, and `jump.onComplete` chains the
rest. Game code can still observe the chain via `OnStateEntered("landing")` if it
needs to spawn a dust puff.

### Interrupt policy

Games need responsive controls: if the player is in the middle of a `run_start`
transition clip and jumps, the jump must start now, not after `run_start` finishes.
So the default is **cut**: a `TransitionTo` arriving during a transition clip
aborts the clip and starts the new edge immediately. A non-default
**interruptible: false** flag on a `TransitionSpec` (deferred) would instead finish
the clip first — needed only for a small class of effects that must not be
truncated.

This differs from `moth::ui::flow`'s default (queue depth 1), because navigation
and character control have opposite priorities: flow transitions are short and
shouldn't be half-applied; character transitions must track high-frequency input.

## Flip and rendering

Flip is a property of `gfx::Sprite` (`SetFlipX`), so `moth::anim` gets it for free
by delegating (`animator.GetSprite().SetFlipX(true)`). The pivot comes from the
frame's `FrameEntry.pivot`, so a flipped sprite stays anchored to the same world
point.

`DrawSprite` renders through `IGraphics::DrawImage(image, transform, pivot, flipX,
flipY)` once the overloads are updated, so flipped sprites draw correctly with no
caller-side UV handling. Callers needing depth sorting either use
`DrawSprite`/`SpriteBatch` or read `GetImage()`/`GetCurrentFrameRect()`/
`GetCurrentFramePivot()` and feed their own batch (the batch's `flipX` field carries
the flip through). The module does not depend on ECS or physics; an entity just
owns an `Animator` and calls `Update` in its update system.

## Data flow summary

```
player.flipbook.json ──SpriteSheetFactory──> gfx::SpriteSheet (frames + clips)
player.anim.json     ──AnimSetLoader───────> AnimSet (states + transitions)
                                                    │
                                  Animator(sheet, set)
                                     │  TransitionTo("run")
                                     ▼
                                 gfx::Sprite ──> IGraphics
                          (flip, speed, clip events)
```

## Validation surface

Like `ValidateFlowGraph`, an `AnimSet` integrity check runs at load and reports all
failures at once. Structural checks (no registries/layouts needed):

- State ids are unique; `initial` exists.
- Every state's `clip` references a clip in the flipbook.
- Every `TransitionSpec`'s `from`/`to` reference defined states.
- No `TransitionSpec` duplicates an existing `from → to` edge.
- A transition `clip` (if present) references a clip in the flipbook and that clip
  is non-looping (a looping transition clip would never complete and would stall
  the state machine).

The last check is a load-time guard against a real footgun. If a transition clip
is `Loop`, the loader fails loudly rather than wedging the character.

## Public API additions

### moth_gfx

- `moth/graphics/graphics/sprite.h` — extended `Sprite` with `SetFlipX`/
  `GetFlipX`, `SetSpeed`/`GetSpeed`, and the `OnClipStarted`/`OnClipStopped`/
  `OnClipLooped` callbacks; `DrawSprite` overloads updated to honour flip.

### moth_anim (new)

New headers under `include/moth/anim/`, all opt-in:

- `moth/anim/anim_set.h` — `AnimSet`, `StateSpec`, `TransitionSpec`, and JSON
  loaders.
- `moth/anim/animator.h` — `Animator` (the state machine).
- `moth/anim/animator_factory.h` — loads `AnimSet` + flipbook via
  `SpriteSheetFactory`, cached by path (parallels `SpriteSheetFactory`).
- `moth/anim/moth_anim.h` — umbrella header.

Dependencies: `moth_anim` → `moth_gfx` (compiled) + `moth_core`. No dependency on
`moth_ui`, `moth_ecs`, or `moth_physics`.

## Resolved decisions

- **Negative / reverse playback — deferred.** `SetSpeed` accepts `> 0` only. A
  negative speed would make the advance loop step *backwards*, which raises
  questions about loop wrap direction (`Loop` wraps to the last step) and about
  `OnClipLooped`/`OnClipStopped` firing at the head rather than the tail. That is
  more than "simple", so it is deferred; revisit when a consumer needs rewind-style
  effects.
- **`SetSpeed(0)` is disallowed.** Pausing is `SetPlaying(false)`, never speed.
  A speed of `0` (or any non-positive value) logs a warning and resets to `1.0f`.
- **No blending.** Transitions are a hard cut, or a one-shot transition clip. No
  cross-fade or blend-tree support.
- **No data-driven conditions.** Whether to transition is entirely game code's
  call. The `AnimSet` carries no predicates, guards, or parameters.
- **No transition parameters.** None in v1; a designer tool would eventually want
  per-edge parameters.
- **`onComplete` fires for `Reset`.** `Stop` and `Reset` both chain via
  `onComplete`; only the visible end frame differs.

## Out of scope for the first cut

- Blend trees / velocity-driven animation blending.
- Reverse playback and per-frame time remapping beyond a uniform speed multiplier.
- Data-driven transition conditions or parameters.
- Skeletal/2D-bone animation; this module is sprite-sheet only.
- Any editor tooling (the data model is kept serialisable to enable one later).
- Integration with `moth::ecs`/`moth::physics` (the `Animator` is deliberately a
  plain object an entity can own, not a component/system pair).

Everything not on this list is v1.
