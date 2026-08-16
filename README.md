# moth_toolkit

A modular, code-first 2D game engine toolkit — a collection of C++ libraries
under the `moth::` namespace that assemble into a complete engine with
compile-time toggles for each feature, while remaining independently usable.

## Status

The `moth_ui` and `moth_graphics` repos are imported (with full history) under
`modules/ui/` and `modules/gfx/`. The `core`, `gfx`, `ui`, `bridge`, `ecs`,
`physics`, `tilemap`, `audio`, and `assets` modules are wired into the superbuild
and Conan-packaged; `moth::ecs` (EnTT-backed entity-component system) replaced
the interim `moth::gfx::scene::Scene`/`Entity` model, `moth::physics` wraps
Box2D, `moth::tilemap` loads and renders Tiled `.tmj` maps, `moth::audio` wraps
miniaudio, and `moth::assets` adds id/path asset addressing plus a `.pak` pack
format (cooked by the `moth_pak` CLI).

## Layout

```
CMakeLists.txt        superbuild: MOTH_ENABLE_* toggles + add_subdirectory
modules/              the moth:: libraries (each Conan-packaged)
  core/                 moth::core    — loop/platform/math/events
  gfx/                  moth::gfx     — 2D renderer + backends (imported)
  ui/                   moth::ui      — node graph/layers/animation (imported)
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

## Building

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Modules build in-tree via the superbuild (gated by `MOTH_ENABLE_*` toggles) and
standalone via their own `conanfile.py` / CMake install-export.

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

## Roadmap

The toolkit grows in phases, each shippable on its own. It currently provides
the core math/event/loop types, the Vulkan-backed `moth::gfx` renderer, the
`moth::ui` node-graph system, an EnTT-backed ECS, Box2D physics, Tiled `.tmj`
tilemaps, miniaudio playback, and id/path asset loading with a `.pak` pack
format. Every module is Conan-packaged and usable on its own or through the
superbuild; unprocessed projects keep loading assets by path, and `moth_pak`
adds optional archive-based loading.
