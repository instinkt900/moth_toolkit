# moth::bridge

**Package** `moth_bridge` · **Namespace** `moth::bridge` · **Headers** `<moth/bridge/application.h>`, `<moth/bridge/ui_window.h>` · **CMake target** `moth::bridge`

The ui ↔ gfx glue. Adapts `moth::ui::IRenderer` onto `moth::gfx::IGraphics`
(`MothRenderer`/`MothImage`/`MothFont`/`MothFlipbook` + factories), composes a
gfx `Window` with a `moth::ui` context + ImGui (`UiWindow`), and provides
`Application`, a UI-driven game loop. Depends on `moth_core` + `moth_graphics` +
`moth_ui`.

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/bridge/application.h>
#include <moth/graphics/platform/glfw/glfw_platform.h>

class MyGame : public moth::bridge::Application {
public:
    MyGame(moth::gfx::platform::IPlatform& platform)
        : Application(platform, "My Game", 1280, 720) {}
    void Startup() override          { /* before the window is created */ }
    void PostCreateWindow() override { /* window + ImGui ready */ }
    void TickFixed(uint32_t ticks) override {
        Application::TickFixed(ticks);   // updates the UI
        /* fixed-step logic */
    }
};

int main() {
    moth::gfx::platform::glfw::Platform platform;
    platform.Startup();
    MyGame game(platform);
    game.Init();
    game.Run();
    platform.Shutdown();
}
```

## Why it exists

`moth::ui` knows nothing about rendering: it draws through `IRenderer` and loads
assets through `IImageFactory`, `IFontFactory`, and `IFlipbookFactory`.
`moth::gfx` knows nothing about UI. This module connects the two, so neither
depends on the other.

Pick the entry point that matches your game:

- **`moth::gfx::game::Game`** (in `moth_graphics`): code-first, with no UI and no
  bridge. The quickest start.
- **`moth::bridge::Application`**: a window with a `moth::ui` layer stack, ImGui,
  and a fixed-timestep loop, for games built around authored UI layouts.
- **`UiWindow` or the adapters on their own**: when you run your own loop.

## Application

`Application` is a `moth::core::Ticker` and an event listener. `Init()` creates
the window, the `UiWindow`, and ImGui. `Run()` runs the loop until the window is
closed.

| Hook | When |
|---|---|
| `Startup()` | Before the window is created |
| `PostCreateWindow()` | After the window and ImGui are ready; push your first layer here |
| `TickFixed(ticks)` | At the fixed update rate (default 60 Hz; `ticks` in ms) |
| `Tick(ticks)` | Once per loop iteration |
| `Shutdown()` | After the loop exits |
| `OnEvent(event)` | Window and input events |

The base implementations do the real work: `TickFixed` updates the `UiWindow`,
`Tick` draws the layer stack and ImGui, and `OnEvent` handles quit requests. If you
override any of them, call the `Application` version as well, or the UI stops
updating, drawing, or closing.

- `GetUiWindow()` and `GetWindow()` return `nullptr` before `Init()`.
- `SetImGuiViewportsEnabled(true)` turns on ImGui multi-viewport support. It must
  be called before `Init()`.

```cpp
void MyGame::PostCreateWindow() {
    GetUiWindow()->PushLayer(std::make_unique<TitleScreenLayer>());
}
```

## UiWindow

`UiWindow` wraps a `moth::gfx::platform::Window` and owns everything the UI needs
on top of it: the `moth::ui::Context`, the `LayerStack`, the renderer and factory
adapters, and the ImGui context.

| Call | Returns / does |
|---|---|
| `GetMothContext()` | The `moth::ui::Context` wired to the adapters |
| `GetLayerStack()` / `PushLayer(layer)` | The UI layer stack |
| `GetGraphics()` / `GetSurfaceContext()` / `GetWindow()` | The underlying graphics window |
| `GetImGuiContext()` / `HasImGuiContext()` / `SetImGuiContext(ctx)` | The ImGui context |
| `Update(ticks)` / `BeginFrame()` / `Draw()` / `EndFrame()` | The per-frame steps, if you drive it yourself |

It installs itself as the window's UI delegate, so input events reach the layer
stack with coordinates mapped to the logical render size.

## Adapters

| Class | Implements | Backed by |
|---|---|---|
| `MothRenderer` | `moth::ui::IRenderer` | `moth::gfx::IGraphics` |
| `MothImage` / `MothImageFactory` | `IImage` / `IImageFactory` | `moth::gfx::TextureFactory` |
| `MothFont` / `MothFontFactory` | `IFont` / `IFontFactory` | `moth::gfx` fonts |
| `MothFlipbook` / `MothFlipbookFactory` | `IFlipbook` / `IFlipbookFactory` | `moth::gfx::SpriteSheet` |

`MothRenderer` translates the UI's push/pop state (blend mode, colour, transform,
clip, texture filter) and its rect, gradient, image, and text calls into
`IGraphics` calls.

## Formatting UI types

`<moth/bridge/moth_ui_format.h>` provides `fmt` formatters for `moth::ui` types:
vectors, rects, `LayoutRect`, and every `moth::ui` enum, formatted by name via
`magic_enum`. They live here to keep `moth_ui` free of formatting dependencies.

```cpp
#include <moth/bridge/moth_ui_format.h>
moth::core::log::info("node rect {}", node->GetScreenRect());   // [(0, 0) -> (100, 200)]
```

## Using the package

```python
def requirements(self):
    self.requires("moth_bridge/0.1.0")
```

```cmake
find_package(moth_bridge REQUIRED)
target_link_libraries(my_game PRIVATE moth::bridge)
```

`moth_core`, `moth_graphics`, and `moth_ui` headers reach consumers transitively.

## Tests

This module has no test suite of its own.
