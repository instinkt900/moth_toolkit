# TODO

## v1.0 Relaunch

**Effort:** Small

The library has never been publicly published and the version number has drifted through internal
development. The two prerequisites — the sprite animation data model redesign and the API contract
tests — are both complete. Reset the version to 1.0.0 and make a deliberate effort to keep the
public API stable across minor versions going forward.

---

## Pre-1.0 API fixes

### `AddChild` index type — `include/moth_ui/nodes/group.h:44`

**Effort:** Trivial

`size_t index = -1` wraps to `SIZE_MAX` at the call site. Change to `int index = -1`.

### Remove mutable `GetColor()` / `GetRotation()` overloads — `include/moth_ui/nodes/node.h:169,196`

**Effort:** Small

`SetRotation()` calls `UpdateLocalTransform()` internally; the mutable `float& GetRotation()`
overload bypasses that, leaving the local transform stale. `Color& GetColor()` is similarly
confusing alongside the proper setter. Remove both mutable overloads.

### Remove `IImage::ImGui()` from the core interface — `include/moth_ui/graphics/iimage.h:29`

**Effort:** Medium

A pure virtual ImGui method on the core image interface forces every backend implementation —
including production, non-editor ones — to depend on ImGui. Remove it from `IImage`. The editor
can cast to its concrete backend type or use a separate opt-in interface.

### `Context` constructor — validate non-null required args — `include/moth_ui/context.h:27`

**Effort:** Trivial

`GetImageFactory()`, `GetFontFactory()`, and `GetRenderer()` use `assert()` which is stripped in
Release builds, causing a silent nullptr dereference if null is passed. Throw
`std::invalid_argument` in the constructor for null `imageFactory`, `fontFactory`, or `renderer`.
`flipbookFactory` is intentionally optional — leave it as-is.

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
