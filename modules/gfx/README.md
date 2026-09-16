# moth::gfx

**Package** `moth_graphics` · **Namespace** `moth::gfx` · **Umbrella** `<moth/graphics/moth_graphics.h>` · **CMake target** `moth_graphics::moth_graphics` (Conan) / `moth::gfx` (superbuild)

The Vulkan-backed 2D renderer. `IGraphics` (immediate-mode draw calls, render
targets, push/pop state, float coords, textured quads, nine-slice, custom
shaders), `IGraphicsDevice` (render-target creation), value types
(`Image`/`ITexture`/`IFont`), `AssetContext` + factories
(texture/font/spritesheet/shader), `Sprite`/`SpriteSheet`/`SpriteBatch`,
`Camera`, and the platform bootstrap (`IPlatform`/`Window`/`ImGuiContext`) that
creates the surface and owns the graphics/device. Also `moth::gfx::game::{Game,
Scene}` for a minimal run loop. Depends on `moth_core` + GLFW/FreeType/HarfBuzz
(system on Linux) + Vulkan, optionally `glslang`.

Part of the [Moth Toolkit](../../README.md). This module doesn't depend on
`moth::ui`; the UI integration lives in [`moth::bridge`](../bridge/README.md).

```cpp
#include <moth/graphics/game/game.h>

using namespace moth::gfx;
using namespace moth::gfx::game;
using namespace moth::core;

class GameScene : public Scene {
    void Update(float dt) override { m_time += dt; }
    void Draw(IGraphics& graphics) override {
        graphics.SetColor(Color{ 0.10f, 0.12f, 0.16f, 1.0f });
        graphics.DrawFillRectF(FloatRect{ { 0, 0 }, { 1280, 720 } });
        graphics.SetColor(Color{ 1.0f, 0.4f, 0.2f, 1.0f });
        graphics.DrawFillRectF(FloatRect{ { 400, 328 }, { 432, 392 } });
    }
    float m_time = 0.0f;
};

int main() {
    Game game{ "Hello Moth", 1280, 720 };
    return game.Run(std::make_unique<GameScene>());
}
```

### AI Disclosure

AI agents (primarily Claude) are used as tools in this project for tasks such as
refactoring, documentation writing, and test implementation. The architecture,
design decisions, and direction of the project are human-driven. This is not a
vibe-coded project.

## Running a game

### Game and Scene

`game::Game` is the shortest path from `main` to a running game. It owns the GLFW
platform and one window, and runs the loop.

| `Scene` hook | When |
|---|---|
| `OnStart()` | Once, before the loop; the window and graphics are ready, so load resources here |
| `Update(dt)` | Every frame; `dt` in seconds |
| `Draw(graphics)` | Every frame, after `Update`; draw in logical pixels |
| `OnStop()` | Once, after the loop; graphics are still valid |

`Game(title, width, height)` throws `std::runtime_error` if the platform or window
can't be created. `Run(scene)` returns the exit code, and `Quit()` ends the loop
after the current frame. `GetWindow()` and `GetGraphics()` give access to the
window and renderer.

### A raw window

Without `Game`, create the platform and window yourself. `IPlatform` gives you a
`platform::Window`, whose `GetGraphics()`/`GetDevice()` expose the renderer and
resource creator, and whose `GetSurfaceContext().GetAssetContext()` loads textures
and fonts.

```cpp
#include <moth/graphics/moth_graphics.h>
#include <moth/graphics/platform/glfw/glfw_platform.h>

moth::gfx::platform::glfw::Platform platform;
platform.Startup();
{
    auto window = platform.CreateWindow("My Game", 1280, 720);
    auto& graphics = window->GetGraphics();

    bool running = true;
    window->AddEventListener([&](moth::core::Event const& event) {
        if (moth::core::event_cast<moth::core::EventRequestQuit>(event) != nullptr) {
            running = false;
            return true;
        }
        return false;
    });

    while (running) {
        window->Update(elapsedMs);          // pump events
        window->BeginFrame();
        graphics.SetLogicalSize({ 1280, 720 });
        // ... draw ...
        window->EndFrame();
    }
}   // destroy the window before shutting the platform down
platform.Shutdown();
```

`glfw::Platform` is the only backend. `CreateImGuiContext` sets up Dear ImGui
(bundled under `external/`) for a window.

## Drawing

`IGraphics` draws immediately, in call order. Coordinates are logical pixels
(`SetLogicalSize`), and the active transform applies to every draw.

| Group | Calls |
|---|---|
| State | `SetColor` / `PushColor` / `PopColor`, `SetBlendMode` / `PushBlendMode` / `PopBlendMode`, `SetTransform` / `PushTransform` / `PopTransform`, `SetClip` / `PushClip` / `PopClip`, `SetShader` |
| Frame | `Clear()`, `Clear(color)`, `SetLogicalSize`, `WaitIdle` |
| Shapes | `DrawRectF`, `DrawFillRectF`, `DrawFillCircleF`, `DrawFillEllipseF`, `DrawFillPolygonF`, `DrawTrianglesF`, `DrawLineF`, `DrawGradientRect` |
| Images | `DrawImage` (transform + pivot + flip, rect, float rect, or position + pivot), `DrawImageTiled`, `DrawImage9Slice`, `DrawImageCircle`, `DrawTexturedTrianglesF` |
| Text | `DrawText(text, font, destRect, horizontal, vertical)` |
| Shaders | `DrawShader(shader)`, `DrawShader(shader, destRect)` |
| Targets | `SetTarget`, `GetTarget` |

`PushTransform` composes on top of the current transform, which is how a camera
transform combines with per-object transforms. `PushClip` intersects with the
enclosing clip.

### Render targets

```cpp
auto target = window->GetDevice().CreateTarget(256, 256);
graphics.SetTarget(target.get());
// ... draw into the target ...
graphics.SetTarget(nullptr);                 // back to the window
graphics.DrawImage(target->GetImage(), IntRect{ { 0, 0 }, { 256, 256 } });
```

## Assets

`AssetContext`, from `window.GetSurfaceContext().GetAssetContext()`, loads GPU
resources:

| Call | Loads |
|---|---|
| `TextureFromFile` / `TextureFromMemory` / `TextureFromPixels` | Textures from files, encoded bytes, or raw RGBA |
| `FontFromFile(path, size)` / `FontFromMemory(bytes, size)` | Fonts at a pixel size |
| `CreateShaderFromGLSL` / `CreateShaderFromSpirV` | Custom fragment shaders |
| `SaveTextureToPNG(texture, path, rect)` | Writes part of a texture to disk |

It also owns cached factories:

| Factory | Use |
|---|---|
| `GetTextureFactory()` | `GetTexture(path)` (cached), `LoadTexturePack(json)` for `moth_packer` atlases, `GetTextureRect(path)`, `SetFallbackTexture` |
| `GetFontFactory()` | `GetFont(path, size)` (cached) |
| `GetSpriteSheetFactory()` | `GetSpriteSheet(path)` for `.flipbook.json` descriptors |
| `GetShaderFactory()` | `CreateFromGLSL`, `CreateFromSpirV`, lookup by name |

`Image` is a cheap value type: a shared texture plus a source rect. An empty
`Image` means "nothing loaded".

```cpp
auto& textures = window.GetSurfaceContext().GetAssetContext().GetTextureFactory();
textures.LoadTexturePack("assets/sprites.json");
auto texture = textures.GetTexture("assets/sprites/player.png");
moth::gfx::Image player{ texture, textures.GetTextureRect("assets/sprites/player.png") };
```

## Sprites

- **`SpriteSheet`** holds an atlas image, per-frame rects and pivots, and named
  clips. A clip is a sequence of frames with durations and a loop type (`Stop`,
  `Reset`, or `Loop`).
- **`Sprite`** plays a sheet: `SetClip`, `SetPlaying`, `Update(ms)`, `SetFrame`,
  horizontal flip, playback speed, and clip started/stopped/looped callbacks. Draw
  it with the `DrawSprite` helpers.
- **`SpriteBatch`** buffers sprites and draws them sorted by `z` (lower first), so
  entities can be layered without ordering the draw calls by hand.

```cpp
auto sheet = assets.GetSpriteSheetFactory().GetSpriteSheet("chars/hero.flipbook.json");
moth::gfx::Sprite hero(sheet);
hero.SetClip("run");
hero.Update(elapsedMs);
moth::gfx::DrawSprite(graphics, hero, IntVec2{ 400, 300 });

moth::gfx::SpriteBatch batch;
moth::gfx::SpriteBatch::Sprite entry;
entry.z = 1.0f;
entry.image = player;
entry.transform = { position, rotation, { 1.0f, 1.0f } };
batch.Add(entry);
batch.Flush(graphics);
```

For state-machine character animation on top of `Sprite`, see
[`moth::anim`](../anim/README.md).

## Camera

`Camera` handles position, zoom, rotation (radians), smoothed follow, and shake.

```cpp
moth::gfx::Camera camera;
camera.Follow(playerPos, dt, 8.0f);     // exponential smoothing; <= 0 snaps
camera.Shake(6.0f, 0.3f);               // intensity, seconds
camera.Update(dt);                      // advances the shake

FloatVec2 const viewport{ 1280, 720 };
graphics.SetTransform(camera.GetViewTransform(viewport));   // includes the shake offset
FloatVec2 const mouseWorld = camera.ScreenToWorld(mouseScreen, viewport);
```

`GetViewportBounds(viewport, topLeft, bottomRight)` returns the visible world
rectangle, for culling. `WorldToScreen`, `Move`, `SnapTo`, `SetZoom`, and
`SetRotation` cover the rest.

## Custom shaders

`moth::gfx` can draw Shadertoy-style fragment shaders, compiled from GLSL at
runtime or loaded as precompiled SPIR-V. While a shader is set with `SetShader`,
every shape and image draw uses it; `DrawText` doesn't. Shaders receive the draw
colour, the shape-local `uv`, `iTime`/`iResolution`/`iMouse`, and up to four
images bound with `Shader::SetChannel(0..3, image)`. See
[Custom shaders](../../README.md#custom-shaders) in the toolkit README and
[`examples/shader_demo/`](../../examples/shader_demo/).

## Build options

| Conan option | CMake option | Default | Effect |
|---|---|---|---|
| `enable_glslang` | `MOTH_GRAPHICS_ENABLE_GLSLANG` | off | Runtime GLSL compilation (adds `glslang` and `spirv-tools`) |
| `disable_vulkan` | `MOTH_GRAPHICS_DISABLE_VULKAN` | off | Strips the Vulkan/GLFW backend |

Vulkan/GLFW is the only backend. `disable_vulkan` leaves nothing to render with;
it exists to keep the backend interface open. The resulting
`MOTH_GRAPHICS_DISABLE_VULKAN` definition reaches every consumer, so guard
backend-specific includes with it:

```cpp
#if !MOTH_GRAPHICS_DISABLE_VULKAN
#include <moth/graphics/platform/glfw/glfw_platform.h>
#endif
```

The built-in shaders ship precompiled as SPIR-V in `resources/`.

### Linux system libraries

On Linux, GLFW, FreeType, and HarfBuzz must come from the system package manager,
not Conan. Mixing Conan-built and system copies of these libraries in one process
causes runtime conflicts through GTK3/GDK-Pixbuf.

```bash
sudo apt install libglfw3-dev libfreetype-dev libharfbuzz-dev
```

On Windows, Conan provides them.

## Using the package

```python
def requirements(self):
    self.requires("moth_graphics/[~2.0]")
```

```cmake
find_package(moth_graphics REQUIRED)
target_link_libraries(my_game PRIVATE moth_graphics::moth_graphics)
```

## Tests

```bash
cd modules/gfx/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

## Known limitations

- **One window per `Game`.** More windows can be created with
  `IPlatform::CreateWindow`, but their lifetime is up to the caller.
- **Linear colour format.** The swapchain and render targets use
  `VK_FORMAT_B8G8R8A8_UNORM`, so blending isn't sRGB-correct. Switching to
  `VK_FORMAT_B8G8R8A8_SRGB` (same byte order) would fix that; see
  `vulkan_graphics_pipeline.cpp`. Typical 2D and UI rendering is unaffected.
- **Linux system libraries.** See [above](#linux-system-libraries).

## See also

- [`examples/hello_game/`](../../examples/hello_game/): the minimal `Game` loop.
- [`examples/sample_game/`](../../examples/sample_game/): input, camera, ECS, and
  transformed sprites.
- [`examples/shader_demo/`](../../examples/shader_demo/): custom shaders.

## License

MIT. See [LICENSE](LICENSE).
