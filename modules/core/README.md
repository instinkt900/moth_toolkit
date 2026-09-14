# moth::core

**Package** `moth_core` · **Namespace** `moth::core` · **Headers** `<moth/core/*.h>` (no umbrella; include individually) · **CMake target** `moth::core`

Always-present foundation. Math/types (`Vector`/`Rect`/`Color`/`Transform2D`/
`BlendMode`/`TextAlignment`/`AABB`/`geometry`/`angle`/`interp`), game math
(`Random`, `PerlinNoise`/`SimplexNoise`), timing (`Timer`/`Stopwatch`/`Cooldown`/
`Tween`), the event system (`Event`/`IEventListener`/`EventDispatch`/
`EventEmitter`), `Ticker` (fixed-timestep loop), pollable `Input`
(key/mouse/gamepad + action map + axes), the abstract `Window`, and the native
GLFW window (`moth::core::glfw::Window` + `PollGamepads`). Depends on
`nlohmann_json`, `fmt`, `spdlog`, and GLFW (system GLFW on Linux).

Part of the [Moth Toolkit](../../README.md). Every other module depends on it.

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
auto& input = Input::Get();              // Input is a process-wide singleton
input.BindAction("jump", Key::Space);    // or a mouse/gamepad button
if (input.IsActionPressed("jump")) { /* ... */ }
```

## Headers

| Area | Headers |
|---|---|
| Math and types | `vector.h`, `vector_utils.h`, `rect.h`, `color.h`, `transform.h`, `transform2d.h`, `aabb.h`, `geometry.h`, `angle.h`, `interp.h`, `blend_mode.h`, `text_alignment.h` |
| JSON conversion | `vector_serialization.h`, `rect_serialization.h` |
| Game math | `random.h`, `noise.h` |
| Timing | `timer.h`, `tween.h`, `ticker.h` |
| Events | `event.h`, `event_listener.h`, `event_dispatch.h`, `event_emitter.h`, `event_key.h`, `event_mouse.h`, `event_window.h` |
| Input | `input.h` |
| Logging | `log.h` |
| Windowing | `window.h`, `glfw/window.h`, `glfw/gamepad.h`, `glfw/events.h` |

There's no umbrella header, so include what each file uses.

## Math and types

- **`Vector<T, Dim>`**, with the aliases `FloatVec2` and `IntVec2`. Components are
  `x`/`y`/`z`/`w`, or `r`/`g`/`b`/`a` for 3 and 4 dimensions. Arithmetic is
  element-wise, and `vector_utils.h` adds `Length`, `Normalized`, `Dot`,
  `Distance`, `Rotate2D`, and friends.
- **`Rect<T>`** (`IntRect`, `FloatRect`) is stored as `topLeft`/`bottomRight`, with
  `x()`, `y()`, `w()`, `h()`. `MakeRect(x, y, w, h)` builds one from a position and
  size.
- **`Color`** is a `Vector<float, 4>` in 0..1. `BasicColors::White`, `Black`,
  `Red`, and so on are predefined.
- **`Transform2D`** holds `position`, `rotation` (radians, clockwise), and `scale`.
  **`FloatMat4x4`** is the matrix form, with `Translation`, `Scale`,
  `Rotation(radians, pivot)`, `Invert`, and `TransformPoint`.
- **`AABB`** is a centre and half-extents, with `FromMinMax`, `Contains`,
  `Overlaps`, `DistanceSq`, `Merged`, and `Expand`.
- **`geometry.h`** has `Circle`, `Segment`, and `Ray`, with overlap tests between
  circles, boxes, rects, and segments, and ray casts against circles, boxes, and
  rects.
- **Angles are radians everywhere.** `DegToRad` and `RadToDeg` convert;
  `WrapAngleRadians`, `AngleDeltaRadians`, and `LerpAngleRadians` handle
  wrap-around.
- **`interp.h`** has 30+ easing curves selected by `InterpType`. Apply one with
  `Interp(a, b, t, type)`.

## Game math

```cpp
Random rng(1234);                        // seedable 64-bit generator
int roll = rng.NextInt(1, 6);            // inclusive
float spread = rng.NextFloat(-0.1f, 0.1f);

SimplexNoise noise(42);
float height = noise.Fractal(x * 0.01f, y * 0.01f, 5);   // octaves; roughly [-1, 1]
```

`Random` also offers `NextFloat()` in [0, 1), `NextBool()`, and `Seed()`, and works
as the engine for the standard distributions. `PerlinNoise` and `SimplexNoise` are
dependency-free 2D samplers with `Noise(x, y)` and fractal octaves. For authored
node-graph noise, use [`moth::noise`](../noise/README.md).

## Timing

All durations are seconds.

```cpp
Timer waveTimer(30.0f);
waveTimer.Update(dt);
if (waveTimer.IsComplete()) { waveTimer.Reset(); }

Cooldown fire(0.25f);
fire.Update(dt);
if (input.IsActionDown("fire") && fire.TryFire()) { SpawnBullet(); }

FloatTween fade(1.0f, 0.0f, 0.5f, InterpType::QuadOut);
fade.Update(dt);
sprite.alpha = fade.GetValue();
```

| Type | Purpose |
|---|---|
| `Timer` | Counts up to a duration: `Update`, `IsComplete`, `Progress`, `GetRemaining`, `Reset`, `Start(duration)` |
| `Stopwatch` | Accumulates time while running: `Start`, `Stop`, `Update`, `Restart`, `GetElapsed` |
| `Cooldown` | Rate limiting: `Update`, `IsReady`, `TryFire` (fires and restarts if ready), `Reset` |
| `Tween<T>` | Eases a value from one end to the other with an `InterpType`; `FloatTween`, `Vec2Tween`, and `ColorTween` are predefined |

### Ticker

`Ticker` is a fixed-timestep loop. Subclass it and implement both callbacks:

- `TickFixed(ticks)` runs at the fixed rate (default 60 Hz), several times in a
  row if the loop fell behind. `ticks` is the fixed interval in milliseconds.
- `Tick(ticks)` runs once per iteration with the leftover milliseconds; render
  here.

`TickSync()` blocks until `SetRunning(false)` is called, usually from an event
handler. After a long stall, such as a debugger pause, the loop drops the backlog
(beyond 100 catch-up ticks) rather than bursting through it.

## Events

- **`Event`** is the polymorphic base. Each concrete event has an integer type
  code, and `event_cast<T>(event)` returns a typed pointer, or `nullptr` if the
  types don't match.
- **`IEventListener`** has one method, `bool OnEvent(Event const&)`. Return `true`
  to consume the event.
- **`EventDispatch`** routes one event to typed handlers and stops after the first
  handler that returns `true`.
- **`EventEmitter`** broadcasts to registered listeners: `AddEventListener`
  (a pointer, or a lambda that returns a `LambdaHandle`), `RemoveEventListener`,
  and `EmitEvent`.

```cpp
bool MyLayer::OnEvent(Event const& event) {
    EventDispatch dispatch(event);
    dispatch.Dispatch(this, &MyLayer::OnKey);          // bool OnKey(EventKey const&)
    dispatch.Dispatch(this, &MyLayer::OnMouseDown);    // bool OnMouseDown(EventMouseDown const&)
    return dispatch.GetHandled();
}
```

Built-in events cover keys (`EventKey`), the mouse (`EventMouseDown`, `Up`,
`Move`, `Wheel`), and the window (`EventWindowSize`, `EventRequestQuit`,
`EventQuit`, `EventRenderDeviceReset`, `EventRenderTargetReset`). Custom events
take type codes from the `EVENTTYPE_USER0`, `USER1`, and `USER2` ranges.

## Input

`Input::Get()` is a process-wide singleton with pollable state. The GLFW window
feeds it, and `glfw::PollGamepads()` updates gamepads. If you run your own
platform layer, call `BeginFrame()` at the start of each frame, then
`ProcessEvent(event)` for each input event. Call `Reset()` on focus loss.

| Query | Calls |
|---|---|
| Keys | `IsKeyDown`, `IsKeyPressed`, `IsKeyReleased` |
| Mouse | `IsMouseButtonDown` / `Pressed` / `Released`, `GetMousePos` (logical coordinates), `GetMouseDelta`, `GetScrollDelta` |
| Gamepads | `IsGamepadConnected`, `IsGamepadButtonDown` / `Pressed` / `Released`, `GetGamepadAxis` |

"Pressed" and "released" are true only on the frame the state changed.

### Actions and axes

Bind names to inputs so game code doesn't hard-code devices:

```cpp
input.BindAction("jump", Key::Space);
input.BindAction("jump", GamepadButton::A);

input.BindAxisKeys("move_x", Key::A, Key::D);
input.BindAxisGamepad("move_x", GamepadAxis::LeftX);

float moveX = input.GetAxis("move_x");                 // [-1, 1]
if (input.IsActionPressed("jump")) { Jump(); }
```

An action is down if any of its bindings is. `BindAxisMouse` binds a pair of mouse
buttons to an axis.

## Logging

The moth libraries log through a replaceable logger. By default messages go to the
console.

```cpp
moth::core::log::info("loaded {} textures", count);
moth::core::log::error("failed to open {}", path.string());
```

- To redirect output, implement `ILogger::Log(level, message)` and register it
  with `SetLogger(&logger)`. The logger must outlive all logging; call
  `SetLogger(nullptr)` before destroying it to go back to the console.
- To silence output, register a `NullLogger`.
- `SetLogger` and `GetLogger` are thread-safe.

## Windowing

`Window` is the abstract native window: `Update(ticks)` pumps events,
`BeginFrame`/`EndFrame` bracket rendering, and it reports its size, position, and
logical render size. It's an `EventEmitter`, so listeners receive its input and
window events.

`glfw::Window` is the GLFW implementation. Games normally get a window from
`moth::gfx`'s platform layer, which adds rendering on top, rather than creating
one directly.

## Build options

| Conan option | CMake option | Default | Effect |
|---|---|---|---|
| `enable_platform` | `MOTH_CORE_ENABLE_PLATFORM` | on | Builds the GLFW windowing and event backend |

Turn it off for tools that only need the types, such as an external editor built
against [`moth::noise`](../noise/README.md) that owns its own window. Doing so
drops the GLFW dependency.

On Linux, GLFW comes from the system package manager, and `pkg-config` is needed
to find it:

```bash
sudo apt install libglfw3-dev pkg-config
```

On Windows, Conan provides GLFW.

## Using the package

```python
def requirements(self):
    self.requires("moth_core/0.1.0")
```

```cmake
find_package(moth_core REQUIRED)
target_link_libraries(my_game PRIVATE moth::core)
```

## Tests

```bash
cd modules/core/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```
