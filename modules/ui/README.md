# moth::ui

**Package** `moth_ui` (2.0.0) · **Namespace** `moth::ui` · **Umbrella** `<moth/ui/moth_ui.h>` (+ `<moth/ui/moth_ui_fwd.h>`) · **CMake target** `moth_ui::moth_ui` (Conan) / `moth::ui` (superbuild)

A node-graph UI system: `Context`, `LayerStack`, `Node` hierarchy, keyframe
animation, and screen flow. It defines renderer-agnostic abstractions
(`IRenderer`/`IImage`/`IFont`/`IFlipbook`) and is decoupled from any backend —
rendering happens through [`moth::bridge`](../bridge/README.md). Depends on
`moth_core` + `nlohmann_json`, `magic_enum`, `range-v3`, `fmt`.

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/ui/moth_ui.h>

using namespace moth::ui;
// Build a Context (node graph + layer stack), populate a Node hierarchy, and
// drive its animation tracks. Rendering is done through moth::bridge's
// MothRenderer, which adapts IRenderer onto moth::gfx's IGraphics.
```

UIs are described in JSON layout files (`.mothui`, edited with
[moth_editor](https://github.com/instinkt900/moth_editor)), loaded at runtime, and
drawn through a small set of interfaces. Features:

- a node-based scene graph: `Group`, `NodeRect`, `NodeImage`, `NodeText`,
  `NodeClip`, `NodeFlipbook`, `NodeGradient`;
- Flash-style keyframe animation with per-property tracks and 30+ easing curves;
- animation markers that fire events during playback, so UI and game can stay in
  sync;
- mouse, keyboard, and custom events that bubble through the tree;
- custom widgets registered by class name (`UiButton` and `UiScrollView` are
  bundled);
- a navigation runtime (flow) for screens, overlays, and their transitions.

### AI Disclosure

AI agents (primarily Claude) are used as tools in this project for tasks such as
refactoring, documentation writing, and test implementation. The architecture,
design decisions, and direction of the project are human-driven. This is not a
vibe-coded project.

## Architecture

| Layer | Responsibility |
|---|---|
| **Layout entities** (`LayoutEntity` subclasses) | The serialisable data model: loaded from JSON, holds keyframe tracks |
| **Nodes** (`Node` subclasses) | The live scene graph: built from layout entities and driven by `AnimationController` |
| **Context** | The asset factories and renderer that every node uses |

A `Context` needs implementations of:

- `IRenderer`: every drawing command;
- `IImageFactory` (`IImage`): image loading;
- `IFontFactory` (`IFont`): font loading;
- `IFlipbookFactory` (`IFlipbook`): flipbook loading. Optional, only needed if
  layouts use flipbook nodes.

In the toolkit, `moth::bridge` provides all of them on top of `moth::gfx`, and its
`UiWindow` builds the `Context` for you. To target another renderer, implement
them yourself.

## Loading a layout

```cpp
moth::ui::Context context(&imageFactory, &fontFactory, &renderer, &flipbookFactory);

auto [root, result] = moth::ui::NodeFactory::Get().Create(context, "layouts/title.mothui", width, height);
if (result != moth::ui::Layout::LoadResult::Success) {
    // DoesNotExist, IncorrectFormat, or InstantiationFailed
}

// Each frame:
root->Update(deltaMs);
root->Draw();
```

The `Context` constructor throws `std::invalid_argument` if the image factory,
font factory, or renderer is null.

`Group` owns children (`AddChild`, `RemoveChild`, `GetChild(id)`, `MoveChild`) and
plays named animation clips. `Node` handles visibility, ids, layout rects
(anchor + offset), screen rects, hit testing, and input capture.

## Layers

A `LayerStack` is an ordered stack of `Layer`s, updated and drawn every frame.
Events go to the top layer first. Subclass `Layer` for each screen or overlay:

| Hook | Use |
|---|---|
| `OnEvent(event)` | Handle input; return `true` to consume it |
| `Update(ticks)` / `Draw()` / `DebugDraw()` | Per-frame logic and rendering |
| `OnAddedToStack(stack)` / `OnRemovedFromStack()` | Lifetime |
| `IsModal()` | Block input and updates to the layers beneath |
| `UseRenderSize()` | Lay out in logical render coordinates rather than window pixels |

With `moth::bridge`, push layers with `UiWindow::PushLayer`.

## Animation and events

Layouts carry animation clips made of per-property keyframe tracks, evaluated by
an `AnimationController` on each node. Clips can hold markers that fire
`EventAnimation` during playback. Clip playback raises
`EventAnimationStarted`/`EventAnimationStopped`, and flipbook nodes raise
`EventFlipbookStarted`/`EventFlipbookStopped`. Easing curves come from
`moth::core` (`InterpType`).

Events use the `moth::core` event system: route them to typed handlers with
`EventDispatch`, and define custom events from the `EVENTTYPE_USER*` ranges.

## Widgets

A widget is a C++ class bound to a layout class name. Derive from the `Widget`
CRTP base and the class registers itself with `NodeFactory` at startup, or call
`NodeFactory::RegisterWidget(className, func)` directly. Layouts that use that
class name then instantiate your type. `UiButton` and `UiScrollView` are bundled.

## Flow

The flow system is an optional navigation runtime. A `FlowGraph` describes layers
(screens and overlays) and the transitions between them, and `Flow` plays their
in/out animations, changes the layer stack, and runs your side effects.
`TransitioningLayer` is a ready-made layer backed by a `.mothui` layout. Apps that
manage their own layers can ignore the `moth/ui/flow/` headers.

- [`docs/flow_system_guide.md`](docs/flow_system_guide.md): how to use it.
- [`docs/flow_system_design.md`](docs/flow_system_design.md): why it works this way.

Code in those guides still uses the pre-toolkit `moth_ui::` namespace spelling;
read it as `moth::ui::`.

## Thread safety

The library follows the usual game pattern: set things up at startup, then drive
the UI from one thread.

| Component | Thread safe? |
|---|---|
| `NodeFactory::Get()` | Yes (function-local static) |
| `NodeFactory::RegisterWidget()` / `Create()` | Yes (guarded by a `std::shared_mutex`) |
| `SetLogger()` / `GetLogger()` | Yes (`std::atomic`, in `moth::core`) |
| `LayoutCache` | Yes (guarded by a `std::mutex`) |
| Node trees and layer stacks (update, draw, event dispatch) | **No**: use one thread |

Register custom widgets from the main thread before starting worker threads.

## Including

- `<moth/ui/moth_ui.h>`: every public type. Use it in `.cpp` files.
- `<moth/ui/moth_ui_fwd.h>`: forward declarations only. Use it in headers.

Individual headers under `moth/ui/animation/`, `nodes/`, `layout/`, `layers/`,
`flow/`, and `widgets/` can also be included directly.

## Using the package

```python
def requirements(self):
    self.requires("moth_ui/2.0.0")
```

```cmake
find_package(moth_ui REQUIRED)
target_link_libraries(my_game PRIVATE moth_ui::moth_ui)
```

## Tests

The test suite uses [Catch2](https://github.com/catchorg/Catch2).

```bash
cd modules/ui/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

## API documentation

If [Doxygen](https://www.doxygen.nl) is installed, configuring the module adds a
`docs` target that writes HTML to `<build>/docs/html/index.html`:

```bash
cmake --build --preset conan-release --target docs
```

## Related projects

| Project | Description |
|---|---|
| [moth_editor](https://github.com/instinkt900/moth_editor) | Visual layout and animation editor for `.mothui` files |
| [moth_packer](https://github.com/instinkt900/moth_packer) | Command-line texture atlas packer for images and layouts |

## License

MIT. See [LICENSE](LICENSE).
