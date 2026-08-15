# Examples

Sample projects and games that exercise the toolkit. Intended to be built
standalone (each with its own CMakeLists) so they double as consumption tests
for the packaged modules.

## `sample_game/`

A single-`main.cpp` game that exercises the Phase 5 P0 features plus the Phase 6
entity-component system:

- **Pollable input** — `moth::core::Input::Get()` (WASD / arrows move the player).
- **Camera** — `moth::gfx::graphics::Camera` follows the player with smoothing.
- **Entity-component system** — `moth::ecs::World` (entities are IDs + components)
  with systems registered in a `moth::ecs::Scheduler`.
- **Float/transform sprite rendering** — a procedurally-generated sprite drawn with
  `IGraphics::DrawImage(image, Transform2D, pivot)`, rotated via a `Transform` component.

### Build in-tree (source superbuild)

```sh
cmake -S . -B build -DMOTH_ENABLE_EXAMPLES=ON
cmake --build build --target sample_game
./build/examples/sample_game/sample_game
```

### Build standalone (packaged modules)

Requires the modules to be in the Conan cache first (see the module `conanfile.py`
recipes, `conan create core` / `conan create gfx` etc.):

```sh
cd examples/sample_game
conan install . --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
```
