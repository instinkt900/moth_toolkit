# moth_ui Technical Review — 2026-04-27

Full-scope review of the public API, source implementations, and test suite. Focus: simplicity, ease of use, documentation quality, and correctness for a C++17 game UI library.

## Implementation Quality

## Testing Gaps

### T1: Completely untested modules
- **Layer / LayerStack** — the entire layer system
- **LayoutCache** — `SetLayoutRoot`, `GetLayout`, `FlushCache`
- **LayoutEntityRef** — sublayout references (Serialize, Deserialize, Instantiate, ReapplyOverrides)
- **LayoutEntityGroup** — standalone serialization
- **LayoutEntityImage** — standalone serialization, nine-slice fields
- **LayoutEntityFlipbook** — standalone serialization
- **LayoutCache** — `SetLayoutRoot`, `GetLayout`, `FlushCache`
- **IFont / IFontFactory** — concrete font usage
- **IImage / IImageFactory** — concrete image usage
- **FontFactory** — concrete implementation
- **Widget\<T\>** — CRTP auto-registration
- **Binary layout serialization** — `.mothb` MessagePack path
- **AnimationTrackController** — direct unit tests
- **AnimationController** — direct unit tests (only tested via fixtures)

### T3: Mock renderer is a no-op
`MockRenderer` has 17 empty methods (`{}`). No recording of calls, no parameter capture. The rendering pipeline is completely untestable at the unit level without an instrumented mock.

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
| T1 | Layer/LayerStack tests | Medium |
| T1 | LayoutCache tests | Small |
| T1 | LayoutEntityRef tests | Medium |

### Medium-term

| # | Issue | Effort |
|---|-------|--------|
| T3 | Instrumented mock renderer | Medium |
| T1 | Binary serialization tests | Small |
| T1 | Widget CRTP tests | Small |
