# moth_ui Technical Review — 2026-04-27

Full-scope review of the public API, source implementations, and test suite. Focus: simplicity, ease of use, documentation quality, and correctness for a C++17 game UI library.

## API Design Issues

### A4: `EventListener` lacks `I` prefix
Every other abstract interface uses `I`-prefix (`IRenderer`, `IImage`, `IFont`, `ILogger`, `IImageFactory`, `IFontFactory`, `IFlipbookFactory`). `EventListener` is the lone exception.

### A5: `m_` prefix on public data members
`AnimationClip`, `AnimationEvent`, `Keyframe`, `LayoutEntity` and its subclasses all expose public members with `m_` prefix. The `m_` prefix conventionally signals private/protected members.

### A6: `Color` exposes full vector arithmetic
`Color` is `Vector<float, 4>` with `operator*`, `operator+`, etc. Component-wise multiplication is not alpha blending, but users might reasonably expect it to be.

### A7: `Layout::Load` uses out-parameter pattern
```cpp
static LoadResult Load(const path&, std::shared_ptr<Layout>* outLayout = nullptr);
```
In C++17 this is antiquated. Consider `std::optional<std::shared_ptr<Layout>>` or a struct return.

### A8: `KeyAction` enum is in `event_key.h` but `Key` enum has no docstring
The `Key` enum (100+ values) has no enum-level docstring. Also `static int constexpr KeyMod_*` should be `static constexpr int`.

### A9: `IFlipbook::LoopType` duplicates `AnimationClip::LoopType`
Both define `Stop`, `Loop`, `Reset` but in different orders and with slightly different semantics.

### A10: `EventType` is the only plain `enum` (not `enum class`)
Every other enum uses `enum class`. `EventType` is a C-style `enum` because it's used as `int` type codes. This is pragmatic but should be documented.

### A11: `Rect` has redundant accessors
`Rect::x()` and `Rect::left()` both return the same value. Same for `y()` and `top()`.

## Documentation Gaps

### D1: `ILogger` and `NullLogger` entirely undocumented
No class-level docstrings, no method docstrings on `Log`, `Debug`, `Info`, `Warning`, `Error`, or `NullLogger::Log`. The `LogLevel` enum is also undocumented.

### D2: `ILogger` template methods force `fmt` dependency
`Debug/Info/Warning/Error` are defined inline in the header and call `fmt::format`. Every includer of `ilogger.h` must have `<fmt/core.h>`.

### D3: Friend `to_json`/`from_json` declarations undocumented
In `AnimationClip`, `AnimationEvent`, `AnimationTrack`, `Keyframe`, `DiscreteAnimationTrack` — none of the friend declarations have docstrings.

### D5: Missing docstrings on special members
Copy/move constructors and destructors on `AnimationTrack`, `LayoutEntity`, `LayoutEntityGroup`, `Node`, `Group`, `Layout` lack docstrings. Several `Layout` overrides (`Clone`, `Instantiate`, `Serialize`, `Deserialize`) are also undocumented.

### D6: `IRenderer::PushTransform` doc says "not composed"
The docstring says transforms are *replaced* per push rather than composed with parent. This contradicts standard transform stack semantics and needs prominent documentation.

## Naming & Consistency

### N1: Constant naming prefix inconsistency
- `k` prefix used: `kDefaultPivot`, `kDegToRad`, `kRadToDeg`
- `k` prefix NOT used: `DefaultFPS`, `DefaultFontSize`, `DefaultBorderSize`
- C-style SCREAMING_CASE: `VersionMajor`, `VersionMinor`, `VersionPatch`, `Version`

### N2: `ClipController` vs `AnimationClipController` naming collision
`ClipController` is a two-pointer struct; `AnimationClipController` is the full playback controller. Consider renaming `ClipController` to `ClipBinding`.

### N3: `AnimationTrack::Target::Events` is a dead enum value
`Events` is "ignored by AnimationController" and "not a float value target." It exists only as a discriminant for serialization but pollutes the target enum.

### N4: `InterpType` naming vs `ease` function prefix
The enum is `InterpType`, the function pointer type is `EaseFunction`, individual functions use `ease` prefix. Pick one scheme.

### N5: `AnimationEvent` vs `EventAnimation` naming
`AnimationEvent` is a marker struct (a named frame). `EventAnimation` is an event class (fires when a marker is crossed). Rename `AnimationEvent` → `AnimationMarker` for clarity.

## Implementation Quality

### I1: `json.contains()` + `json["key"]` double lookup
`src/layout/layout.cpp:75-76, 79-81, 83-88` and `src/font_factory.cpp:26-31` — `contains()` followed by `operator[]` or `at()` does two hash lookups. Use `json.find()` or `json.value()` with defaults.

### I2: `json["mothui_version"]` uses `operator[]` which inserts null on missing key
`src/layout/layout.cpp:66` — If the version key is absent, `operator[]` inserts a null value. Use `json.value("mothui_version", 0)`.

### I3: `LayoutEntityRef` serializes then re-parses override JSON
`src/layout/layout_entity_ref.cpp:84, 104` — Overrides are `dump()`'d to string then `parse()`'d back. Store the `nlohmann::json` object directly.

### I4: `NodeFlipbook::SharedFromThis` uses `dynamic_pointer_cast` unnecessarily
`src/nodes/node_flipbook.cpp:187` — `dynamic_pointer_cast<NodeFlipbook>(shared_from_this())` when `this` is statically `NodeFlipbook*`. Use `std::static_pointer_cast<NodeFlipbook>`.

### I7: Renderer state push/pop has no RAII guards
`Node::Draw()` and `NodeText::DrawInternal()` manually call `PushColor`/`PopColor` etc. A scoped RAII guard would provide exception safety.

### I8: Logger raw pointer with no invalidation mechanism
`src/logger.cpp:6-20` — `s_logger` is `std::atomic<ILogger*>`. If the pointed-to logger is destroyed, `GetLogger()` returns a dangling reference.

## Testing Gaps

### T1: Completely untested modules
- **Layer / LayerStack** — the entire layer system
- **LayoutCache** — `SetLayoutRoot`, `GetLayout`, `FlushCache`
- **LayoutEntityRef** — sublayout references (Serialize, Deserialize, Instantiate, ReapplyOverrides)
- **LayoutEntityGroup** — standalone serialization
- **LayoutEntityImage** — standalone serialization, nine-slice fields
- **LayoutEntityFlipbook** — standalone serialization
- **EventKey** — key event construction and accessors
- **IFont / IFontFactory** — concrete font usage
- **IImage / IImageFactory** — concrete image usage
- **FontFactory** — concrete implementation
- **Widget\<T\>** — CRTP auto-registration
- **Binary layout serialization** — `.mothb` MessagePack path
- **AnimationTrackController** — direct unit tests
- **AnimationController** — direct unit tests (only tested via fixtures)

### T2: Critical untested error paths
- `AnimationTrack::GetOrCreateKeyframe` with negative frame
- `Node::SetEventHandler` with null function
- `AnimationClip` with `m_startFrame > m_endFrame` (negative FrameCount)

### T3: Mock renderer is a no-op
`MockRenderer` has 17 empty methods (`{}`). No recording of calls, no parameter capture. The rendering pipeline is completely untestable at the unit level without an instrumented mock.

### T5: `static_assert` inside `TEST_CASE` is misleading
`api_surface_animation.cpp` uses `static_assert` inside `TEST_CASE` bodies. `static_assert` fires at compile time regardless of test selection. Move to namespace scope.

## What's Good

- Entity/node split is the right architecture for Flash-like animation
- Enum-driven animation targets (not string-based)
- API surface compile-time contract tests (8 files) — more projects should do this
- NodeFlipbook test suite is thorough (790 lines, error paths, loop types, event dedup)
- Interpolation test suite is systematic (all 30+ easing functions at boundaries, symmetry checks)
- No vendored dependencies (pure Conan)
- C++17 throughout, conventional commits + git-cliff CI hygiene
- Most public API has docstrings
- Clean separation of continuous vs. discrete animation tracks
- LayoutRect anchor/offset model is well-designed for resolution independence
- `LayoutCache` double-checked locking pattern is correctly implemented
- `NodeFactory` singleton uses C++11 magic statics for thread safety
- All items from the previous review (2026-04-25) are resolved

## Prioritized Action Plan

### Short-term

| # | Issue | Effort |
|---|-------|--------|
| D1 | `ILogger`/`NullLogger` documentation | Medium |
| D3 | Friend `to_json`/`from_json` documentation | Small |
| D5 | Special member docstrings | Medium |
| A7 | `Layout::Load` return type modernization | Medium |
| T1 | Layer/LayerStack tests | Medium |
| T1 | LayoutCache tests | Small |
| T1 | LayoutEntityRef tests | Medium |
| T1 | EventKey tests | Small |
| T2 | Remaining critical untested error path tests | Small |
| N5 | `AnimationEvent` → `AnimationMarker` rename | Medium |

### Medium-term

| # | Issue | Effort |
|---|-------|--------|
| A4 | `EventListener` → `IEventListener` rename (or convention documentation) | Large (breaking) |
| A5 | `m_` prefix on public members — document or change | Large (breaking) |
| A6 | `Color` vector arithmetic misuse protection | Medium |
| T3 | Instrumented mock renderer | Medium |
| T1 | Binary serialization tests | Small |
| T1 | Widget CRTP tests | Small |
| A10 | Document `EventType` plain-enum rationale | Trivial |
| N1 | Constant naming convention standardization | Small |
| N3 | `AnimationTrack::Target::Events` removal | Medium |
| I3 | `LayoutEntityRef` JSON store-don't-serialize | Small |
| I7 | RAII renderer state guards | Small |
