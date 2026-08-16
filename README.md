# moth_toolkit

A modular, code-first 2D game engine toolkit — a collection of C++ libraries
under the `moth::` namespace that assemble into a complete engine with
compile-time toggles for each feature, while remaining independently usable.

## Status

The `moth_ui` and `moth_graphics` repos are imported (with full history) under
`ui/` and `gfx/`. The `core`, `gfx`, `ui`, `bridge`, `ecs`, `physics`, `tilemap`,
and `audio` modules are wired into the superbuild and Conan-packaged; `moth::ecs`
(EnTT-backed entity-component system) replaced the interim
`moth::gfx::scene::Scene`/`Entity` model, `moth::physics` wraps Box2D,
`moth::tilemap` loads and renders Tiled `.tmj` maps, and `moth::audio` wraps
miniaudio. See the Obsidian vault roadmap for the remaining work (Phase 7 DX).

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
  bridge/               moth::bridge  — ui <-> gfx adapter
  toolkit/              moth::toolkit — aggregate target + feature header
cmake/features.h.in   generated MOTH_HAS_* compile-time flags
examples/             sample games / consumption tests
tools/                moth new scaffold CLI + template
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

## Plan

The full design lives in the Obsidian vault (`Moth_Toolkit/`), covering vision,
architecture, feature gaps, the renderer review, the roadmap, open decisions,
and the monorepo migration guide.
