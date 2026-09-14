# moth::toolkit

**Package** `moth_toolkit` · **Umbrella** `<moth/toolkit.h>` · **CMake target** `moth::toolkit` (superbuild) / `moth_toolkit::moth_toolkit` (Conan)

The aggregate meta-package. Links every module you enable and pulls in one header
that includes them all, guarded by `MOTH_ENABLE_*` flags. Its Conan `enable_*`
options turn modules on/off (defaults: all on), validated so a module can't be
enabled without its dependencies. Depends on whatever you enable.

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/toolkit.h>

#if MOTH_ENABLE_GFX
    // use moth::gfx
#endif
#if MOTH_ENABLE_PHYSICS
    // use moth::physics
#endif
```

This package has no code of its own: it's a header plus dependency wiring. Every
module can also be used on its own through its own package.

## Conan options

```ini
# conanfile.txt
[requires]
moth_toolkit/0.1.0

[options]
moth_toolkit/*:enable_audio=False
moth_toolkit/*:enable_physics=False
```

| Option | Module | Requires |
|---|---|---|
| `enable_core` | `moth_core` | — |
| `enable_gfx` | `moth_graphics` | core |
| `enable_ui` | `moth_ui` | core |
| `enable_bridge` | `moth_bridge` | core, gfx, ui |
| `enable_ecs` | `moth_ecs` | core |
| `enable_physics` | `moth_physics` | core |
| `enable_tilemap` | `moth_tilemap` | core, gfx |
| `enable_audio` | `moth_audio` | core |
| `enable_assets` | `moth_assets` | core |
| `enable_anim` | `moth_anim` | core, gfx |
| `enable_net` | `moth_net` | core |
| `enable_noise` | `moth_noise` | core |

All options default to `True`. A configuration that enables a module without its
requirements is rejected with a message naming the option to change.

Each enabled module is required with transitive headers and libs, and listed as a
component requirement. Linking the single `moth_toolkit::moth_toolkit` target
therefore pulls in every enabled module, and the modules' own targets
(`moth::physics` and so on) are still available.

## Feature flags

`<moth/toolkit.h>` includes each enabled module's headers, guarded by a
`MOTH_ENABLE_<MODULE>` flag (0 or 1). The flags come from one of two places:

- **Conan package:** compile definitions on the `moth_toolkit` target.
- **Superbuild:** the generated `<moth/features.h>`, which the header picks up
  through `__has_include`.

A flag that isn't defined anywhere defaults to 1, matching the superbuild
defaults, so the header can be included on its own.

The generated `<moth/features.h>` also defines friendlier `MOTH_HAS_*` aliases
(plus `MOTH_HAS_TOOLKIT`), but only in the superbuild. The Conan package defines
only `MOTH_ENABLE_*`, and an undefined macro in `#if` quietly evaluates to 0. Use
`MOTH_ENABLE_*` in code that has to build both ways.

## What the umbrella header includes

| Flag | Headers |
|---|---|
| `MOTH_ENABLE_CORE` | The commonly used core headers: vector, rect, color, log, transforms, AABB, geometry, angle, random, noise, blend mode, text alignment, interp, timer, tween, events, ticker, window, input |
| `MOTH_ENABLE_GFX` | `<moth/graphics/moth_graphics.h>` |
| `MOTH_ENABLE_UI` | `<moth/ui/moth_ui.h>` |
| `MOTH_ENABLE_ECS` | `<moth/ecs/ecs.h>` |
| `MOTH_ENABLE_PHYSICS` | `<moth/physics/physics.h>` |
| `MOTH_ENABLE_TILEMAP` | `<moth/tilemap/tilemap.h>` |
| `MOTH_ENABLE_AUDIO` | `<moth/audio/audio.h>` |
| `MOTH_ENABLE_ASSETS` | `<moth/assets/assets.h>`, `<moth/assets/pak.h>` |
| `MOTH_ENABLE_ANIM` | `<moth/anim/moth_anim.h>` |
| `MOTH_ENABLE_NET` | `<moth/net/net.h>` |
| `MOTH_ENABLE_NOISE` | `<moth/noise/noise.h>` |
| `MOTH_ENABLE_BRIDGE` | application, UI window, renderer, image, font, and flipbook headers |

Including everything costs compile time. In larger projects, include the specific
module headers each file needs and keep `<moth/toolkit.h>` for small tools or
prototypes.

Headers outside the module umbrellas are not included, such as
`<moth/tilemap/tile_map_physics.h>` and `<moth/core/glfw/*.h>`.

## Superbuild

In the source superbuild the same aggregate is the `moth::toolkit` interface
target, controlled by `-DMOTH_ENABLE_<MODULE>=ON/OFF` and
`-DMOTH_ENABLE_TOOLKIT=ON/OFF`. See [Building](../../README.md#building) in the
toolkit README.
