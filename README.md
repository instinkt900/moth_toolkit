# Moth Toolkit

A modular, code-first 2D game engine toolkit — a collection of C++ libraries
under the `moth::` namespace that assemble into a complete engine with
compile-time toggles for each feature, while remaining independently usable.

Each module is a standalone Conan package with its own version and dependencies;
the `moth::toolkit` meta-package aggregates them into a single dependency you can
turn on/off feature by feature.

## Table of Contents

- [Status](#status)
- [Modules](#modules)
  - [moth::core](#mothcore)
  - [moth::gfx](#mothgfx)
  - [moth::ui](#mothui)
  - [moth::bridge](#mothbridge)
  - [moth::ecs](#mothecs)
  - [moth::physics](#mothphysics)
  - [moth::tilemap](#mothtilemap)
  - [moth::audio](#mothaudio)
  - [moth::assets](#mothassets)
  - [moth::toolkit](#mothtoolkit)
- [Using the toolkit as a whole](#using-the-toolkit-as-a-whole)
- [Building](#building)
- [Starting a new game](#starting-a-new-game)
- [Packing assets](#packing-assets)
- [Custom shaders](#custom-shaders)
- [Layout](#layout)
- [Roadmap](#roadmap)

## Status

The `moth_ui` and `moth_graphics` repos are imported (with full history) under
`modules/ui/` and `modules/gfx/`. The `core`, `gfx`, `ui`, `bridge`, `ecs`,
`physics`, `tilemap`, `audio`, and `assets` modules are wired into the superbuild
and Conan-packaged; `moth::ecs` (EnTT-backed entity-component system) replaced
the interim `moth::gfx::scene::Scene`/`Entity` model, `moth::physics` wraps
Box2D, `moth::tilemap` loads and renders Tiled `.tmj` maps, `moth::audio` wraps
miniaudio, and `moth::assets` adds id/path asset addressing plus a `.pak` pack
format (cooked by the `moth_pak` CLI).

## Modules

Every module is a Conan package (`moth_*`) and a CMake target (`moth::*`).
Package versions live in each module's `modules/*/version.txt` (read by its
`conanfile.py` at create time); `moth_graphics` and `moth_ui` are `2.0.0`, the
rest are `0.1.0`. Use a module on its own by depending on its package, or use the
whole toolkit through `moth_toolkit` (see [below](#using-the-toolkit-as-a-whole)).

### moth::core

**Package** `moth_core` · **Namespace** `moth::core` · **Headers** `<moth/core/*.h>` (no umbrella; include individually)

Always-present foundation. Math/types (`Vector`/`Rect`/`Color`/`Transform2D`/
`BlendMode`/`TextAlignment`/`AABB`/`geometry`/`angle`/`interp`), game math
(`Random`, `PerlinNoise`/`SimplexNoise`), timing (`Timer`/`Stopwatch`/`Cooldown`/
`Tween`), the event system (`Event`/`IEventListener`/`EventDispatch`/
`EventEmitter`), `Ticker` (fixed-timestep loop), pollable `Input`
(key/mouse/gamepad + action map + axes), the abstract `Window`, and the native
GLFW window (`moth::core::glfw::Window` + `PollGamepads`). Depends on
`nlohmann_json`, `fmt`, `spdlog`, and GLFW (system GLFW on Linux).

```cpp
#include <moth/core/vector.h>
#include <moth/core/rect.h>
#include <moth/core/color.h>
#include <moth/core/ticker.h>
#include <moth/core/input.h>

using namespace moth::core;

FloatVec2 velocity = { 3.0f, -1.5f };
IntRect  viewport = MakeRect(0, 0, 1280, 720);
Color    tint     = Color{ 1.0f, 0.4f, 0.2f, 1.0f };

Ticker ticker(60);                       // 60 Hz fixed loop (TickFixed/Tick)
Input  input;
input.BindAction("jump", Key::Space);    // or a mouse/gamepad button
if (input.IsActionPressed("jump")) { /* ... */ }
```

### moth::gfx

**Package** `moth_graphics` · **Namespace** `moth::gfx` · **Umbrella** `<moth_graphics/moth_graphics.h>`

The Vulkan-backed 2D renderer. `IGraphics` (immediate-mode draw calls between
`Begin`/`End`, render targets, push/pop state, float coords, textured quads,
nine-slice, custom shaders), `IGraphicsDevice` (render-target creation), value
types (`Image`/`ITexture`/`IFont`), `AssetContext` + factories
(texture/font/spritesheet/shader), `Sprite`/`SpriteSheet`/`SpriteBatch`,
`Camera`, and the platform bootstrap (`IPlatform`/`Window`/`ImGuiContext`) that
creates the surface and owns the graphics/device. Also `moth::gfx::game::{Game,
Scene}` for a minimal run loop. Depends on `moth_core` + GLFW/FreeType/HarfBuzz
(system on Linux) + Vulkan, optionally `glslang`.

```cpp
#include <moth_graphics/game/game.h>

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

For a raw (non-`game::Game`) window, `platform::IPlatform` gives you a
`platform::Window` whose `GetGraphics()`/`GetDevice()` expose the renderer and
resource creator; `SurfaceContext::GetAssetContext()` loads textures/fonts.

### moth::ui

**Package** `moth_ui` · **Namespace** `moth::ui` · **Umbrella** `<moth_ui/moth_ui.h>`

A node-graph UI system: `Context`, `LayerStack`, `Node` hierarchy, keyframe
animation, and screen flow. It defines renderer-agnostic abstractions
(`IRenderer`/`IImage`/`IFont`/`IFlipbook`) and is decoupled from any backend —
rendering happens through `moth::bridge` (see below). Depends on `moth_core` +
`nlohmann_json`, `magic_enum`, `range-v3`, `fmt`.

```cpp
#include <moth_ui/moth_ui.h>

using namespace moth::ui;
// Build a Context (node graph + layer stack), populate a Node hierarchy, and
// drive its animation tracks. Rendering is done through moth::bridge's
// MothRenderer, which adapts IRenderer onto moth::gfx's IGraphics.
```

### moth::bridge

**Package** `moth_bridge` · **Namespace** `moth::bridge` · **Headers** `<moth/bridge/application.h>`, `<moth/bridge/ui_window.h>`

The ui ↔ gfx glue. Adapts `moth::ui::IRenderer` onto `moth::gfx::IGraphics`
(`MothRenderer`/`MothImage`/`MothFont`/`MothFlipbook` + factories), composes a
gfx `Window` with a `moth::ui` context + ImGui (`UiWindow`), and provides
`Application`, a UI-driven game loop. Depends on `moth_core` + `moth_graphics` +
`moth_ui`.

```cpp
#include <moth/bridge/application.h>
#include <moth_graphics/platform/glfw/glfw_platform.h>

class MyGame : public moth::bridge::Application {
public:
    MyGame(moth::gfx::platform::IPlatform& platform)
        : Application(platform, "My Game", 1280, 720) {}
    void Startup() override          { /* before the window is created */ }
    void PostCreateWindow() override { /* window + ImGui ready */ }
    void TickFixed(uint32_t ticks) override { /* fixed-step logic */ }
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

### moth::ecs

**Package** `moth_ecs` · **Namespace** `moth::ecs` · **Umbrella** `<moth/ecs/ecs.h>`

A header-only, EnTT-backed entity-component system: `World` (a thin
`entt::registry` wrapper), core components (`Transform`/`Active`/`Tag`), and a
`Scheduler` for ordered systems. Depends on `moth_core` + `entt`.

```cpp
#include <moth/ecs/ecs.h>

using namespace moth::ecs;
World world;
Entity e = world.Create();
world.Emplace<Active>(e);                 // core component (or your own types)
if (world.Has<Active>(e)) { world.Get<Active>(e).value = false; }

Scheduler update;
update.Add([](World& w, float dt) { /* system */ });
update.Run(world, 1.0f / 60.0f);
```

### moth::physics

**Package** `moth_physics` · **Namespace** `moth::physics` · **Umbrella** `<moth/physics/physics.h>`

A Box2D 2.4.1 wrapper: `World` (gravity, step, body create/destroy, contact
listener, AABB/ray queries) plus `ToB2`/`FromB2` vector helpers. Bodies,
fixtures, shapes, and forces are Box2D's own types. Depends on `moth_core` +
`box2d`.

```cpp
#include <moth/physics/physics.h>

using namespace moth::physics;
World world({ 0.0f, -10.0f });          // gravity

b2BodyDef bodyDef;
bodyDef.type = b2_dynamicBody;
bodyDef.position = b2Vec2{ 0.0f, 5.0f };
b2Body* body = world.CreateBody(bodyDef);

b2CircleShape shape;
shape.m_radius = 0.5f;
b2FixtureDef fixture;
fixture.shape = &shape;
fixture.density = 1.0f;
body->CreateFixture(&fixture);

world.Step(1.0f / 60.0f);               // advance the simulation
```

### moth::tilemap

**Package** `moth_tilemap` · **Namespace** `moth::tilemap` · **Umbrella** `<moth/tilemap/tilemap.h>`

Grid-based maps from Tiled `.tmj`: `TileMap`/`Tileset`/`Layer`/`TileId`, a TMJ
importer (embedded tilesets, CSV + base64, flip flags), culled layered rendering
via `IGraphics`, and a world ↔ tile + query API. Depends on `moth_core` +
`moth_graphics` + `nlohmann_json` + `zlib`.

```cpp
#include <moth/tilemap/tilemap.h>

using namespace moth::tilemap;
TileMap map = LoadTileMapFromFile("level.tmj");   // or LoadTileMap(jsonText)

std::vector<moth::gfx::Image> tilesets;           // one Image per tileset
DrawTileMap(graphics, map, tilesets, viewRect);   // culled, layered draw
```

### moth::audio

**Package** `moth_audio` · **Namespace** `moth::audio` · **Umbrella** `<moth/audio/audio.h>`

A miniaudio wrapper: `AudioEngine` (device/node graph, master volume, null-device
mode) and `Sound` (decoded one-shot or streamed music; play/pause/stop/volume/
looping/pitch/seek/length). Depends on `moth_core` + `miniaudio`.

```cpp
#include <moth/audio/audio.h>

using namespace moth::audio;
AudioEngine engine;                       // AudioEngineConfig{ .nullDevice } for headless
engine.Start();

Sound blip  = engine.LoadSound("blip.wav");
Sound music = engine.LoadMusic("theme.ogg");
music.SetVolume(0.4f);
music.Play();
blip.Play();
```

### moth::assets

**Package** `moth_assets` · **Namespace** `moth::assets` · **Umbrella** `<moth/assets/assets.h>` (+ `<moth/assets/pak.h>`)

Id/path asset addressing and a `.pak` pack format. Each asset's id is the FNV-1a
hash of its relative path, so it can be addressed by id or path. Cook a folder
with `moth_pak`, then load the archive at runtime via `PackedAssetSource` and
feed the bytes to any `...FromMemory` loader. Depends on `moth_core` +
`nlohmann_json`.

```cpp
#include <moth/assets/assets.h>
#include <moth/assets/pak.h>

auto source = moth::assets::PackedAssetSource::Load("data.pak");
auto bytes = source.Read(idOrPath);       // by AssetId or path string
engine.LoadSoundFromMemory(bytes);        // any FromMemory loader
```

### moth::toolkit

**Package** `moth_toolkit` · **Umbrella** `<moth/toolkit.h>`

The aggregate meta-package. Links every module you enable and pulls in one header
that includes them all, guarded by `MOTH_ENABLE_*` / `MOTH_HAS_*` flags. Its
Conan `enable_*` options turn modules on/off (defaults: all on), validated so
enabling a module enables its dependencies. Depends on whatever you enable.

```cpp
#include <moth/toolkit.h>

#if MOTH_HAS_GFX
    // use moth::gfx
#endif
#if MOTH_HAS_PHYSICS
    // use moth::physics
#endif
```

## Using the toolkit as a whole

Two ways to consume everything at once:

- **CMake superbuild** — the root `CMakeLists.txt` builds every enabled module
  as one project (see [Building](#building)).
- **Conan meta-package** — depend on `moth_toolkit` and select features with its
  `enable_*` options; it re-exports each module's CMake targets.

```python
# conanfile.txt
[requires]
moth_toolkit/0.1.0

[options]
moth_toolkit/*:enable_audio=False
moth_toolkit/*:enable_physics=False
```

Then link `moth::toolkit` (or the individual `moth::*` targets) and
`#include <moth/toolkit.h>`. Feature flags are available to consumer code as
`MOTH_ENABLE_*` and the friendlier `MOTH_HAS_*` aliases (from the generated
`<moth/features.h>` in the superbuild, or a compile definition from Conan).

## Building

### Superbuild (source, in-tree)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Enable or disable modules with `-DMOTH_ENABLE_*=ON/OFF`:

| Option | Default | Controls |
|---|---|---|
| `MOTH_ENABLE_CORE` | ON | `moth::core` (always required by the rest) |
| `MOTH_ENABLE_GFX` | ON | `moth::gfx` renderer |
| `MOTH_ENABLE_UI` | ON | `moth::ui` node graph |
| `MOTH_ENABLE_BRIDGE` | ON | `moth::bridge` ui↔gfx adapter |
| `MOTH_ENABLE_ECS` | ON | `moth::ecs` ECS |
| `MOTH_ENABLE_PHYSICS` | ON | `moth::physics` Box2D |
| `MOTH_ENABLE_TILEMAP` | ON | `moth::tilemap` Tiled maps |
| `MOTH_ENABLE_AUDIO` | ON | `moth::audio` miniaudio |
| `MOTH_ENABLE_ASSETS` | ON | `moth::assets` addressing + `.pak` |
| `MOTH_ENABLE_TOOLKIT` | ON | `moth::toolkit` aggregate target |
| `MOTH_ENABLE_TOOLS` | OFF | the `moth_pak` CLI |
| `MOTH_ENABLE_EXAMPLES` | OFF | the example projects |

For example, a renderer-only build:

```bash
cmake -S . -B build -DMOTH_ENABLE_UI=OFF -DMOTH_ENABLE_BRIDGE=OFF \
     -DMOTH_ENABLE_ECS=OFF -DMOTH_ENABLE_PHYSICS=OFF \
     -DMOTH_ENABLE_TILEMAP=OFF -DMOTH_ENABLE_AUDIO=OFF \
     -DMOTH_ENABLE_ASSETS=OFF -DMOTH_ENABLE_TOOLKIT=OFF
cmake --build build
```

### Conan packages (per module)

Each `modules/*` is a standalone Conan recipe (version read from its
`version.txt`). Build one:

```bash
cd modules/gfx
conan create . --build=missing -s build_type=Release
```

Consume it from another project by `conan install`-ing against a `conanfile`
that `requires` the package, then use the generated `conan-release` preset:

```bash
conan install . --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
```

The `moth_graphics` package optionally enables runtime GLSL compilation with
`-o enable_glslang=True` (and `-DMOTH_GRAPHICS_ENABLE_GLSLANG=ON` in the
superbuild).

## Starting a new game

Scaffold a project from the bundled template:

```bash
python3 tools/moth_new.py my_game
cd my_game
conan install . --build=missing -s build_type=Release
cmake --preset conan-release && cmake --build --preset conan-release
```

That drops you into a compiling game loop using `moth::gfx::game::Game`:

```cpp
int main() {
    moth::gfx::game::Game game{ "My Game", 1280, 720 };
    return game.Run(std::make_unique<MyScene>());
}
```

See `examples/` for fuller samples (ECS sprites, physics, tilemaps, audio).

## Packing assets

Assets load by path by default. To cook a folder into a single `.pak` archive
plus a `manifest.json`, use `moth_pak` (built with `-DMOTH_ENABLE_TOOLS=ON`):

```bash
./build/tools/moth_pak/moth_pak assets/ --pak out/assets.pak --manifest out/manifest.json
```

Each asset's id is the FNV-1a hash of its path relative to the input folder, so
it can later be addressed by id or by path. Load the archive at runtime with
`moth::assets::PackedAssetSource`, then feed the bytes to any `...FromMemory`
loader (e.g. `AudioEngine::LoadSoundFromMemory`). See
`examples/packed_audio_demo/` for a complete cook → load-by-id → play loop.

## Custom shaders

`moth::gfx` can draw Shadertoy-style fragment shaders. Compile GLSL at runtime
(opt-in, needs glslang) or load precompiled SPIR-V, then set it as the active
shader and draw shapes as usual:

```cpp
auto shader = assetContext.GetShaderFactory().CreateFromGLSL("plasma", R"GLSL(
    void mainImage(out vec4 fragColor, in vec2 fragCoord) {
        vec2 uv = fragCoord / iResolution.xy;
        fragColor = vec4(0.5 + 0.5 * cos(iTime + uv.xyx + vec3(0, 2, 4)), 1.0);
    }
)GLSL");

graphics.SetShader(shader.get());          // every draw now uses this shader
graphics.DrawFillRectF(rect);              // rasterised by the shader
graphics.DrawFillCircleF(center, radius);  // likewise
graphics.SetShader(nullptr);               // back to the default shader
```

The shader receives the interpolated vertex colour (`SetColor`), the shape-local
`uv` (0..1), and the Shadertoy built-ins `iTime`/`iResolution`/`iMouse`. Bind up
to four images with `Shader::SetChannel(0..3, image)` (`iChannel0..3`); `DrawImage`
also honours the shader and binds its image as `iChannel0`. `DrawText` ignores the
active shader. Runtime GLSL compilation is off by default — enable it with
`-DMOTH_GRAPHICS_ENABLE_GLSLANG=ON` (and `-o enable_glslang=True` for Conan). See
`examples/shader_demo/` for a full sample.

## Layout

```
CMakeLists.txt        superbuild: MOTH_ENABLE_* toggles + add_subdirectory
modules/              the moth:: libraries (each Conan-packaged)
  core/                 moth::core    — loop/platform/math/events
  gfx/                  moth::gfx     — 2D renderer + backends
  ui/                   moth::ui      — node graph/layers/animation
  ecs/                  moth::ecs     — EnTT entity-component system
  physics/              moth::physics — Box2D rigid bodies
  tilemap/              moth::tilemap — Tiled .tmj maps + tilesets
  audio/                moth::audio   — miniaudio sound + music
  assets/               moth::assets  — id/path addressing + .pak format
  bridge/               moth::bridge  — ui <-> gfx adapter
  toolkit/              moth::toolkit — aggregate target + feature header
cmake/features.h.in   generated MOTH_HAS_* compile-time flags
examples/             sample games / consumption tests
tools/                moth new scaffold CLI + moth_pak asset cooker
```

