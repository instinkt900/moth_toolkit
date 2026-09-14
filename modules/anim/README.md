# moth::anim

**Package** `moth_anim` · **Namespace** `moth::anim` · **Umbrella** `<moth/anim/moth_anim.h>` · **CMake target** `moth::anim`

Data-driven character animation. `AnimSet`/`StateSpec`/`TransitionSpec` describe
states and transitions in JSON (each state maps to a `.flipbook.json` clip), and
`Animator` drives a `gfx::Sprite` through them — including one-shot transition
clips and automatic `onComplete` chaining (`jump` → `landing` → `idle`).
`gfx::Sprite` itself gains horizontal flip, playback speed, and clip
start/stop/loop callbacks. Depends on `moth_core` + `moth_graphics`.

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/anim/moth_anim.h>

moth::anim::Animator animator(sheet, set);   // set loaded from .anim.json
animator.TransitionTo("run");
animator.Update(elapsedMs);
animator.GetSprite().SetFlipX(facingLeft);
```

## The model

A character has named **states** (`idle`, `run`, `jump`, …), and each state plays
one clip from a flipbook. Game code asks for a state, and the animator plays the
right clip.

- **Edges are implicit.** Any state can move to any other with an instant cut to
  the target's clip.
- **A `TransitionSpec`** exists only to attach a one-shot transition clip to a
  specific edge, for example a `run_start` clip between `idle` and `run`.
- **`onComplete`** chains states: when a non-looping clip finishes, the animator
  enters the named state. Without it, the sprite stays on the last frame.

The animation set describes states only. Frames, clip timing, and loop types live
in the `.flipbook.json` it references (produced by `moth_packer`).

## `.anim.json`

```json
{
    "initial": "idle",
    "flipbook": "player.flipbook.json",
    "states": [
        { "id": "idle",    "clip": "idle" },
        { "id": "run",     "clip": "run" },
        { "id": "jump",    "clip": "jump",    "onComplete": "landing" },
        { "id": "landing", "clip": "landing", "onComplete": "idle" }
    ],
    "transitions": [
        { "id": "idle.run", "from": "idle", "to": "run", "clip": "run_start" }
    ]
}
```

| Field | Meaning |
|---|---|
| `initial` | State entered when the animator is constructed |
| `flipbook` | Path to the `.flipbook.json`, relative to this file |
| `states[].id` / `clip` | Unique state id and the flipbook clip it plays |
| `states[].onComplete` | Optional state to enter when a non-looping clip ends |
| `transitions[].id` | Readable edge name, conventionally `from.to` |
| `transitions[].from` / `to` | The edge |
| `transitions[].clip` | Optional one-shot clip played before the target state |

`ParseAnimSet(jsonText)` checks the structure: unique state ids, an existing
`initial` state, transitions that reference defined states, and no duplicate
edges. It returns `std::nullopt` on failure and logs why. Checks that need the
flipbook (clips exist, loop types fit) happen in `AnimatorFactory`.

## AnimatorFactory

`AnimatorFactory` loads an `.anim.json` and its flipbook through a
`gfx::AssetContext`, caches both by canonical path, and returns a fresh `Animator`
with its own playback state on each call.

```cpp
moth::anim::AnimatorFactory factory(window.GetSurfaceContext().GetAssetContext());

std::shared_ptr<moth::anim::Animator> hero = factory.CreateAnimator("chars/hero.anim.json");
if (!hero) { /* details were logged */ }

factory.FlushCache();   // release cached sets and sprite sheets
```

## Animator

| Call | Does |
|---|---|
| `Animator(sheet, set)` | Constructs and enters `set.initial` |
| `TransitionTo(stateId)` | Switches state, playing the edge's transition clip if one is authored |
| `GetCurrentState()` | The active state (a pending target isn't current until its transition clip ends) |
| `Update(ms)` | Advances playback |
| `GetSprite()` | The underlying `gfx::Sprite`, for drawing, flip, and speed |

Calling `TransitionTo` with the current state restarts its clip and fires the
state callbacks again. Check `GetCurrentState()` first if you don't want that.

### Callbacks

```cpp
hero->OnStateEntered = [](std::string_view state) { /* e.g. play a sound */ };
hero->OnStateExited = [](std::string_view state) {};
hero->OnTransitionStarted = [](std::string_view transitionId) {};
hero->OnTransitionCompleted = [](std::string_view transitionId) {};

hero->OnClipStarted = [](std::string_view clip) {};
hero->OnClipStopped = [](std::string_view clip) {};
hero->OnClipLooped = [](std::string_view clip) {};
```

The animator uses the sprite's own callback slots for sequencing and forwards the
clip callbacks. When a sprite is driven by an animator, hook the **animator's**
callbacks, not the sprite's.

## Drawing

The animator only updates the sprite. Draw it with the usual `moth::gfx` helpers:

```cpp
hero->Update(elapsedMs);
hero->GetSprite().SetFlipX(velocity.x < 0.0f);
moth::gfx::DrawSprite(graphics, hero->GetSprite(), position);
```

## Using the package

```python
def requirements(self):
    self.requires("moth_anim/0.1.0")
```

```cmake
find_package(moth_anim REQUIRED)
target_link_libraries(my_game PRIVATE moth::anim)
```

## Tests

```bash
cd modules/anim/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

## See also

- [`docs/design.md`](docs/design.md): the design of record, including the
  `gfx::Sprite` additions this module builds on.
