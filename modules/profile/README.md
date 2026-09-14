# moth::profile

**Package** `moth_profile` · **Namespace** `moth::profile` · **Headers** `<moth/profile/profiler.h>`, `<moth/profile/imgui/profiler_panel.h>` · **CMake targets** `moth::profile`, `moth::profile_imgui`

A frame profiler: time nested scopes with an RAII object, keep a history of
recent frames, and inspect them in an ImGui panel. The recorder
(`moth::profile`) has no dependencies. The panel (`moth::profile_imgui`) draws
with the ImGui built into `moth_graphics`.

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/profile/profiler.h>
#include <moth/profile/imgui/profiler_panel.h>

moth::profile::ProfilerPanel panel;     // keeps the panel's selection between frames

void GameLayer::Update(uint32_t ticks) {
    MOTH_PROFILE_SCOPE("Update");
    m_world.Update(ticks);
}

void GameLayer::Draw() {
    MOTH_PROFILE_FRAME();               // once per frame
    {
        MOTH_PROFILE_SCOPE("Draw world");
        m_world.Draw();
    }
    panel.Draw();                       // between ImGui::NewFrame and ImGui::Render
}
```

## Headers

| Header | Target | Contents |
|---|---|---|
| `profiler.h` | `moth::profile` | `Profiler`, `ProfileScope`, `FrameRecord`, `ScopeRecord`, and the `MOTH_PROFILE_*` macros |
| `imgui/profiler_panel.h` | `moth::profile_imgui` | `ProfilerPanel` |

## Recording

`MarkFrame()` (or `MOTH_PROFILE_FRAME()`) completes the frame in progress and
starts the next. Call it once per loop iteration. A frame's duration runs from
one mark to the next, so it includes time that no scope covers, such as
presenting and waiting for vsync.

`MOTH_PROFILE_SCOPE("name")` times the rest of the enclosing block. Scopes nest,
and each records its name, depth, start time within the frame, and duration.
Names must outlive the profiler, so use string literals.

- The history keeps the last 300 frames (`Profiler(historySize)` to change).
  `GetFrame(0)` is the oldest.
- Only the thread that first calls `MarkFrame` is recorded. Marks and scopes on
  other threads are ignored.
- Scopes opened before the first mark or while paused are not recorded. A scope
  still open at a mark is cut off at the mark.
- `SetPaused` takes effect at the next mark, so the frame in progress completes.
- Recording stops allocating once the history is full and each frame's scope
  list has reached its usual size.

The macros use the process-wide `Profiler::Get()`. Construct a `Profiler` and
pass it to `ProfileScope` to record separately.

Define `MOTH_PROFILE_DISABLE` to compile `MOTH_PROFILE_SCOPE` and
`MOTH_PROFILE_FRAME` out of a build.

## Panel

`ProfilerPanel::Draw(title)` draws one ImGui window. Call it between
`ImGui::NewFrame` and `ImGui::Render`, for example from a layer's `Draw`.

- **Showing and hiding:** the panel starts open. `SetOpen`, `ToggleOpen` and
  `IsOpen` control it, for example from a key binding. While the panel is
  closed, `Draw` does nothing. The window's close button also closes it.
  Recording goes on while the panel is closed.

- **Frame graph:** one bar per frame in the history. Bars over the budget are
  red, and the line marks the budget (16.7 ms by default; edit it in the
  window). Hover a bar for its frame time.
- **Inspecting a frame:** click a bar, or press **Worst frame**, to pause
  recording and show that frame. **Latest** follows the newest frame again, and
  the **Pause** checkbox resumes recording.
- **Scope tree:** the shown frame's scopes with their time in ms, share of the
  frame, and call count. Sibling scopes with the same name are merged into one
  row. `(untracked)` is the part of the frame that no top-level scope covers.

## Using the package

```python
def requirements(self):
    self.requires("moth_profile/0.1.0")
```

```cmake
find_package(moth_profile REQUIRED)
target_link_libraries(my_game PRIVATE moth::profile moth::profile_imgui)
```

Set the Conan option `with_imgui=False` to build only the recorder, without the
`moth_graphics` dependency. In the superbuild, `MOTH_ENABLE_PROFILE` builds the
module and the panel is built only when `MOTH_ENABLE_GFX` is on.

## Tests

The tests cover the recorder only, so they need no graphics stack.

```bash
cd modules/profile/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```
