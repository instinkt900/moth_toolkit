# TODO

## v1.0 Relaunch

**Effort:** Small

The library has never been publicly published and the version number has drifted through internal
development. The two prerequisites — the sprite animation data model redesign and the API contract
tests — are both complete. Reset the version to 1.0.0 and make a deliberate effort to keep the
public API stable across minor versions going forward.

---

## Pre-1.0 API fixes

---

## Pre-1.0 docstring fixes

### `SetAnimation()` / `StopAnimation()` no-op base — `include/moth_ui/nodes/node.h:203,206`

The no-op defaults on the base `Node` class are intentional (leaf nodes have no clips), but
undocumented. Add a note: "Base implementation is a no-op; `Group` overrides this to drive the
`AnimationClipController`."

### `AnimationController` lifetime invariant — `include/moth_ui/animation/animation_controller.h:25`

The raw `Node*` is always owned by the same node (created in `Node`'s constructor, destroyed with
it), but the invariant is undocumented. Add: "The node must outlive this controller."

### `NodeClip` usage — `include/moth_ui/nodes/node_clip.h:9`

"exists to communicate clip region information through the layout system" does not explain how.
Clarify: the parent `Group` reads children of type `NodeClip` to determine the scissor rectangle
applied before drawing other siblings.

### `EventFlipbookStarted` empty `clipName` — `include/moth_ui/events/event_flipbook.h:15`

"or empty if no clip is set" is surprising for a "started" event. Clarify the scenario:
`Play()` was called while no clip was loaded, so playback begins in a blank state.

### `LayoutCache` concurrent-load note — `include/moth_ui/layout/layout_cache.h:18`

The class docstring claims all methods are thread-safe but does not mention that concurrent calls
with the same name may each load from disk independently; the first insertion wins and both callers
receive the same `shared_ptr`. Document this behaviour.
