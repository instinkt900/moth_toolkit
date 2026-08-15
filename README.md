# moth_toolkit

A modular, code-first 2D game engine toolkit — a collection of C++ libraries
under the `moth::` namespace that assemble into a complete engine with
compile-time toggles for each feature, while remaining independently usable.

## Status

Early scaffolding. The `moth_ui` and `moth_graphics` repos have been imported
(with full history) under `ui/` and `gfx/`. The module extraction (core,
bridge) and toolkit aggregation are placeholders pending the roadmap work.

## Layout

```
CMakeLists.txt        superbuild: MOTH_ENABLE_* toggles + add_subdirectory
core/                 moth::core    — loop/platform/math/events (placeholder)
gfx/                  moth::gfx     — 2D renderer + backends (imported)
ui/                   moth::ui      — node graph/layers/animation (imported)
bridge/               moth::bridge  — ui <-> gfx adapter (placeholder)
toolkit/              moth::toolkit — aggregate target + feature header
cmake/features.h.in   generated MOTH_HAS_* compile-time flags
examples/             sample games / consumption tests
```

## Building

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Only the placeholder modules build for now; `gfx/` and `ui/` still build
standalone (from their own directories) until they are wired into the
superbuild.

## Plan

The full design lives in the Obsidian vault (`Moth_Toolkit/`), covering vision,
architecture, feature gaps, the renderer review, the roadmap, open decisions,
and the monorepo migration guide.
