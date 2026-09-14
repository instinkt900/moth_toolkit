# moth::physics

**Package** `moth_physics` · **Namespace** `moth::physics` · **Umbrella** `<moth/physics/physics.h>` · **CMake target** `moth::physics`

A Box2D 2.4.1 wrapper: `World` (gravity, step, body create/destroy, contact
listener, AABB/ray queries) plus `ToB2`/`FromB2` vector helpers. Bodies,
fixtures, shapes, and forces are Box2D's own types. Depends on `moth_core` +
`box2d`.

Part of the [Moth Toolkit](../../README.md).

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

## Design

The wrapper is deliberately thin. `World` owns a `b2World` and exposes the parts
of Box2D a game loop touches. Everything else (bodies, fixtures, shapes, joints,
forces, filtering) is plain Box2D, so the
[Box2D 2.4 documentation](https://box2d.org/documentation/) applies directly.

If you only need overlap tests and ray casts, not a dynamics simulation, use the
dependency-free `moth::core` geometry (`AABB`, `Circle`, `Segment`, `Ray`) instead.

## World

| Call | Does |
|---|---|
| `World(gravity = { 0, -10 })` | Creates the world. Move-only |
| `Step(dt, velocityIterations = 8, positionIterations = 3)` | Advances the simulation by `dt` seconds |
| `CreateBody(def)` / `DestroyBody(body)` | Creates a body owned by the world / destroys it with its fixtures and joints |
| `SetGravity(g)` / `GetGravity()` | Changes gravity immediately |
| `SetContactListener(listener)` | Installs a `b2ContactListener` |
| `QueryAABB(callback, aabb)` | Calls a `b2QueryCallback` for every fixture whose AABB overlaps |
| `RayCast(callback, p1, p2)` | Calls a `b2RayCastCallback` for each hit |
| `Raw()` | The underlying `b2World` |

The contact listener is **borrowed, not owned**. It must outlive the world, or be
reset with `SetContactListener(nullptr)` before it's destroyed.

```cpp
class Contacts : public b2ContactListener {
    void BeginContact(b2Contact* contact) override { /* ... */ }
};

Contacts contacts;
world.SetContactListener(&contacts);
```

## Units and coordinates

Box2D works in metres, kilograms, and seconds, and its y axis points up. The
wrapper does no conversion: `FloatVec2` values pass straight through as `b2Vec2`
(`ToB2` / `FromB2`). Scale between pixels and metres, and flip y, in your own code.
Box2D is tuned for moving objects roughly 0.1–10 m in size.

## Tilemap collisions

`moth::tilemap` can turn Tiled collision shapes into Box2D fixtures through its
optional `<moth/tilemap/tile_map_physics.h>` header. `moth_tilemap` doesn't depend
on Box2D; a project that uses both modules gets Box2D from this package.

## Using the package

```python
def requirements(self):
    self.requires("moth_physics/0.1.0")
```

```cmake
find_package(moth_physics REQUIRED)
target_link_libraries(my_game PRIVATE moth::physics)
```

Box2D headers are part of this module's public API, so they reach consumers
automatically.

## Tests

The test project builds `moth_physics` from source and covers the world, bodies,
contacts, and collision filtering.

```bash
cd modules/physics/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

## See also

- [`examples/physics_demo/`](../../examples/physics_demo/)
