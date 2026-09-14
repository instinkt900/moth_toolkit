# moth::ecs

**Package** `moth_ecs` · **Namespace** `moth::ecs` · **Umbrella** `<moth/ecs/ecs.h>` · **CMake target** `moth::ecs`

A header-only, EnTT-backed entity-component system: `World` (a thin
`entt::registry` wrapper), core components (`Transform`/`Active`/`Tag`), and a
`Scheduler` for ordered systems. Depends on `moth_core` + `entt` (`~3.15`).

Part of the [Moth Toolkit](../../README.md).

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

## Concepts

- **Entities** are opaque ids (`Entity`, an alias for `entt::entity`).
  `kNullEntity` means "no entity".
- **Components** are plain data structs. Any type can be a component; empty
  structs work as tags and EnTT stores them without an instance.
- **Systems** are plain functions or lambdas taking `(World&, float dt)`, with
  `dt` in seconds.

## World

`World` owns the entity and component storage.

| Call | Does |
|---|---|
| `Create()` / `Destroy(e)` / `Valid(e)` | Create, destroy (with all components), or check an entity |
| `Emplace<T>(e, args...)` | Add a component; returns `T&` (`void` for empty tag types) |
| `Get<T>(e)` | Access a component (undefined if absent) |
| `TryGet<T>(e)` | Access a component, or `nullptr` if absent |
| `GetOrEmplace<T>(e, args...)` | Access a component, adding it first if absent |
| `Has<T>(e)` / `Remove<T>(e)` | Check or remove a component |
| `View<A, B>()` | Iterate entities that have every listed component |
| `View<A>(Exclude<Dead>)` | Same, skipping entities that have any excluded component |
| `Each<A, B>(func)` | Call `func` for every match; `func` may take the entity first |
| `Size()` / `Clear()` | Live entity count / destroy everything |
| `Raw()` | The underlying `entt::registry`, for anything the wrapper doesn't expose |

```cpp
struct Velocity { moth::core::FloatVec2 value; };
struct Dead {};

for (auto [entity, transform, velocity] : world.View<Transform, Velocity>(Exclude<Dead>).each()) {
    transform.transform.position += velocity.value * dt;
}

world.Each<Transform>([](Entity entity, Transform& transform) { /* ... */ });
```

## Core components

| Component | Field | Meaning |
|---|---|---|
| `Transform` | `moth::core::Transform2D transform` | Position, rotation, and scale; rendering systems read it to place sprites |
| `Active` | `bool value = true` | Systems should skip entities whose `value` is false |
| `Tag` | `std::string name` | A readable name for debugging and lookups |

## Scheduler

`Scheduler` runs systems in the order they were registered. `Add` returns the
scheduler, so calls chain. Keep separate schedulers for update and draw when the
phases run at different points in the frame.

```cpp
Scheduler update;
update.Add(MovementSystem)
      .Add(CollisionSystem)
      .Add([](World& w, float dt) { /* inline system */ });

update.Run(world, dt);   // every frame
update.Size();           // number of registered systems
update.Clear();          // remove them all
```

## Using the package

```python
def requirements(self):
    self.requires("moth_ecs/0.1.0")
```

```cmake
find_package(moth_ecs REQUIRED)
target_link_libraries(my_game PRIVATE moth::ecs)
```

## Tests

```bash
cd modules/ecs/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

## See also

- [`examples/sample_game/`](../../examples/sample_game/) uses `World` and `Scheduler`
  with input, a camera, and sprite rendering.
