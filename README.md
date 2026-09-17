# Moth Toolkit

[![CI](https://github.com/instinkt900/moth_toolkit/actions/workflows/ci.yml/badge.svg)](https://github.com/instinkt900/moth_toolkit/actions/workflows/ci.yml)
[![Release](https://github.com/instinkt900/moth_toolkit/actions/workflows/release.yml/badge.svg)](https://github.com/instinkt900/moth_toolkit/actions/workflows/release.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A modular, code-first 2D game engine toolkit — a collection of C++ libraries
under the `moth::` namespace that assemble into a complete engine with
compile-time toggles for each feature, while remaining independently usable.

Each module is a standalone Conan package with its own version and dependencies;
the `moth::toolkit` meta-package aggregates them into a single dependency you can
turn on/off feature by feature.

## Table of Contents

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
  - [moth::anim](#mothanim)
  - [moth::net](#mothnet)
  - [moth::noise](#mothnoise)
  - [moth::profile](#mothprofile)
  - [moth::packer](#mothpacker)
  - [moth::toolkit](#mothtoolkit)
- [Using the toolkit as a whole](#using-the-toolkit-as-a-whole)
- [Building](#building)
- [Tools](#tools)
  - [moth_create](#moth_create)
  - [moth_new](#moth_new)
  - [moth_pak](#moth_pak)
  - [moth_packer](#moth_packer)
- [Related Projects](#related-projects)
- [License](#license)

## Modules

Every module is a Conan package (`moth_*`) and a CMake target (`moth::*`).
Each module has its own version in `modules/*/version.txt`, which its
`conanfile.py` reads at create time. Use a module on its own by depending on its
package, or use the whole toolkit through `moth_toolkit` (see
[below](#using-the-toolkit-as-a-whole)).

The summaries below say what each module is for. Each module's README has the
full API, examples, and build options.

### moth::core

The foundation that every other module builds on. It has the basic math types
(vectors, rectangles, colours, transforms), random numbers and simple noise,
timers and tweens, events, a fixed-timestep game loop, keyboard/mouse/gamepad
input, and the native window.

→ [`modules/core/README.md`](modules/core/README.md)

### moth::gfx

The 2D renderer, built on Vulkan. It draws shapes, images, text, and sprites,
supports render targets, cameras, and custom shaders, and loads textures and
fonts. It also has `moth::gfx::game::Game`, a minimal run loop and the quickest
way to get a window on screen.

→ [`modules/gfx/README.md`](modules/gfx/README.md)

#### Custom shaders

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

### moth::ui

A UI system for menus, HUDs, and screens. You lay out a tree of nodes, stack
them in layers, animate them with keyframes, and move between screens. It
doesn't draw anything itself; `moth::bridge` connects it to `moth::gfx`.

→ [`modules/ui/README.md`](modules/ui/README.md)

### moth::bridge

The glue between `moth::ui` and `moth::gfx`. It renders UI through the gfx
renderer and provides `Application`, a game loop with a UI and ImGui built in.
Use it when your game uses `moth::ui`.

→ [`modules/bridge/README.md`](modules/bridge/README.md)

### moth::ecs

An entity-component system built on [EnTT](https://github.com/skypjack/entt).
Create entities, attach components to them, and run systems over them in a set
order.

→ [`modules/ecs/README.md`](modules/ecs/README.md)

### moth::physics

2D rigid-body physics built on [Box2D](https://box2d.org/). It manages the
physics world (gravity, stepping, collisions, ray and area queries) and uses
Box2D's own types for bodies and shapes.

→ [`modules/physics/README.md`](modules/physics/README.md)

### moth::tilemap

Loads and draws tile maps made in the [Tiled](https://www.mapeditor.org/) map
editor (`.tmj`), including tilesets, animated tiles, object layers, and custom
properties. It can also turn tile collision shapes into physics bodies.

→ [`modules/tilemap/README.md`](modules/tilemap/README.md)

### moth::audio

Sound effects and music built on [miniaudio](https://miniaud.io/). Load a sound
and play, pause, loop, or change its volume and pitch.

→ [`modules/audio/README.md`](modules/audio/README.md)

### moth::assets

Finds game assets by path or by a numeric id, and reads them from loose files or
from a single `.pak` archive (made with the [`moth_pak`](#moth_pak) tool).

→ [`modules/assets/README.md`](modules/assets/README.md)

### moth::anim

Character animation driven by data. Describe animation states (idle, run, jump)
and the transitions between them in JSON, and an `Animator` plays the right
sprite clips.

→ [`modules/anim/README.md`](modules/anim/README.md)

### moth::net

Simple networking for multiplayer games: a TCP server and client that send JSON
messages. It has no background threads; you call `Poll()` once per frame.

→ [`modules/net/README.md`](modules/net/README.md)

### moth::noise

Advanced noise generation built on [FastNoise2](https://github.com/Auburn/FastNoise2),
for terrain and other procedural content. Noise graphs are saved in a readable
JSON format and loaded at runtime. For a quick Perlin or Simplex sample,
`moth::core` is enough.

→ [`modules/noise/README.md`](modules/noise/README.md)

### moth::profile

A frame profiler. Mark the frames and code blocks you want to time, then look at
the results in an ImGui panel with a frame-time graph.

→ [`modules/profile/README.md`](modules/profile/README.md)

### moth::packer

Packs many small images into one texture atlas or flipbook sheet, and extracts
sprites from a sheet. The [`moth_packer`](#moth_packer) tool uses it.

→ [`modules/packer/README.md`](modules/packer/README.md)

### moth::toolkit

A single package that includes all the other modules. Turn each module on or off
with an option, and include one header, `<moth/toolkit.h>`, to get all of them.

→ [`modules/toolkit/README.md`](modules/toolkit/README.md)

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

Then link the aggregate target (`moth::toolkit` in the superbuild,
`moth_toolkit::moth_toolkit` from Conan) or the individual module targets, and
`#include <moth/toolkit.h>`. Feature flags reach consumer code as `MOTH_ENABLE_*`:
a compile definition from Conan, or the generated `<moth/features.h>` in the
superbuild. That generated header also defines friendlier `MOTH_HAS_*` aliases,
but the Conan package doesn't, and an undefined macro in `#if` quietly evaluates
to 0. Use `MOTH_ENABLE_*` in code that has to build both ways.

## Building

### Superbuild (source, in-tree)

The root `conanfile.py` is a dependency provisioner: `conan install` fetches the
third-party libraries and writes the CMake toolchain + deps into
`build/<build_type>/generators`, along with a `conan-release` preset. Install the
dependencies first, then build through the preset:

```bash
conan install . --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
```

C++17 is the minimum. On Linux, Conan's detected profile already uses `gnu17`. On
Windows, MSVC's detected profile defaults to C++14, so pass `-s compiler.cppstd=17`
or set it in your Conan profile.

Enable or disable modules with `-DMOTH_ENABLE_*=ON/OFF` (pass them to the
configure preset):

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
| `MOTH_ENABLE_ANIM` | ON | `moth::anim` character animation |
| `MOTH_ENABLE_NET` | ON | `moth::net` TCP messaging |
| `MOTH_ENABLE_NOISE` | ON | `moth::noise` FastNoise2 node graphs |
| `MOTH_ENABLE_PROFILE` | ON | `moth::profile` frame profiler (the ImGui panel is built only with gfx) |
| `MOTH_ENABLE_PACKER` | ON | `moth::packer` atlas/flipbook packing |
| `MOTH_ENABLE_TOOLKIT` | ON | `moth::toolkit` aggregate target |
| `MOTH_ENABLE_TOOLS` | OFF | the `moth_pak` and `moth_packer` CLIs (needs assets, packer and ui) |
| `MOTH_ENABLE_EXAMPLES` | OFF | the example projects |

For example, a renderer-only build:

```bash
cmake --preset conan-release -DMOTH_ENABLE_UI=OFF -DMOTH_ENABLE_BRIDGE=OFF \
     -DMOTH_ENABLE_ECS=OFF -DMOTH_ENABLE_PHYSICS=OFF \
     -DMOTH_ENABLE_TILEMAP=OFF -DMOTH_ENABLE_AUDIO=OFF \
     -DMOTH_ENABLE_ASSETS=OFF -DMOTH_ENABLE_TOOLKIT=OFF
cmake --build --preset conan-release
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

## Tools

The toolkit comes with a few command-line tools. The Python scripts run from the
toolkit root. The C++ tools are built by the superbuild when you pass
`-DMOTH_ENABLE_TOOLS=ON`, and end up under `build/Release/tools/`.

| Tool | What it does |
|---|---|
| [`moth_create`](#moth_create) | Builds every module into your local Conan cache |
| [`moth_new`](#moth_new) | Creates a new game project that builds out of the box |
| [`moth_pak`](#moth_pak) | Packs a folder of assets into one `.pak` archive |
| [`moth_packer`](#moth_packer) | Packs images into texture atlases or flipbook sheets |

### moth_create

Other projects, including games made with [`moth_new`](#moth_new), don't build
the toolkit from source. They get the `moth_*` packages from your local Conan
cache, so those packages must be built there first. `tools/moth_create.py` does
this for you: it runs `conan create` on every module in dependency order, so
each module is built after the modules it needs.

Run it once after you clone the toolkit, and again whenever you change the
toolkit's sources, so your other projects get the new code.

```bash
python3 tools/moth_create.py                      # all modules, Release
python3 tools/moth_create.py core gfx ui          # just these (still ordered)
python3 tools/moth_create.py -s build_type=Debug  # a debug cache
python3 tools/moth_create.py -r conancenter       # restrict to one remote
python3 tools/moth_create.py --list               # print the order and stop
```

It stops at the first failure, since every later module depends on the ones
before it; pass `--keep-going` to build the rest anyway. The common `conan
create` flags (`-s`, `-o`, `-pr`, `-r`, `-nr`, `--build`) are accepted directly
and anything else goes after a `--` separator. If Conan lives in a virtualenv
rather than on your `PATH`, point at it with `--conan /path/to/conan`.

The module list in that script is the single source of truth for build order —
the CI workflows read it rather than keeping their own copy, and it refuses to
run if a module in `modules/` is missing from it.

### moth_new

`tools/moth_new.py` creates a new game project from the bundled template. The
project uses the packaged `moth_graphics` module (and, through it, `moth_core`),
so put those packages in your local Conan cache first with
[`moth_create`](#moth_create):

```bash
python3 tools/moth_create.py core gfx
```

Then scaffold a project and build it:

```bash
python3 tools/moth_new.py my_game                 # or --dir PATH to put it elsewhere
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

### moth_pak

Assets load by path by default. `moth_pak` cooks a folder of assets into a
single `.pak` archive plus a `manifest.json`:

```bash
./build/Release/tools/moth_pak/moth_pak assets/ --pak out/assets.pak --manifest out/manifest.json
```

Each asset's id is the FNV-1a hash of its path relative to the input folder, so
it can later be addressed by id or by path. Load the archive at runtime with
`moth::assets::PackedAssetSource`, then feed the bytes to any `...FromMemory`
loader (e.g. `AudioEngine::LoadSoundFromMemory`). See
`examples/packed_audio_demo/` for a complete cook → load-by-id → play loop.

### moth_packer

`moth_packer` cooks loose images into texture atlases or a flipbook sheet, and
can pull sprites back out of a sheet:

```bash
# an atlas from a directory of sprites
./build/Release/tools/moth_packer/moth_packer -d sprites/ -r -o out/ sprites

# a flipbook from numbered frames
./build/Release/tools/moth_packer/moth_packer --pack-type flipbook -d frames/ -o out/ run

# pull individual sprites back out of a sheet
./build/Release/tools/moth_packer/moth_packer --mode unpack sheet.png -o out/
```

Input can also come from a file list (`-i`), a glob (`-g`), or the images a
`moth::ui` layout references (`-l` / `-x`). Each run writes the atlas image(s)
plus a JSON descriptor; `--help` lists every option. To do the same from code —
which is what tools built on the toolkit should do — link `moth::packer` and call
`Pack`/`PackToMemory` directly; see
[`modules/packer/README.md`](modules/packer/README.md).

## Related Projects

Editors built on the toolkit, each in its own repository:

| Project | Description |
|---|---|
| [moth_editor](https://github.com/instinkt900/moth_editor) | Visual layout and keyframe animation editor for `moth::ui` layouts |
| [moth_sprite](https://github.com/instinkt900/moth_sprite) | Sprite sheet and animation clip editor whose projects `moth::gfx` loads as a `SpriteSheet` |
| [moth_noised](https://github.com/instinkt900/moth_noised) | Node editor for `moth::noise` graphs, forked from FastNoise2's editor |

## License

The Moth Toolkit is released under the [MIT License](LICENSE).
