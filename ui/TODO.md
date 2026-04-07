# TODO

### Sprite Animation Data Model Redesign

**Effort:** Medium

The current flipbook runtime model assumes a uniform grid — fixed frame size, clips defined by a
start/end frame index range. This needs to be replaced with a more expressive model:

- **Packed texture** — the atlas is a bin-packed image where frames can be any size, each
  described by a bounding rect rather than a grid cell.
- **Frame list** — the descriptor contains an ordered array of named frames, each with a rect
  (x, y, w, h) into the atlas and a pivot point for alignment.
- **Named clips** — clips are named sequences of `{ frame_index, duration }` pairs rather than
  `{ start, end }` ranges. This allows repeated frames, non-linear sequences, and frame sharing
  between clips without duplicating image data.

This is the runtime-side counterpart to the moth_packer sprite animation redesign and the
moth_editor sprite editor (see their respective TODOs). All three need to move together. The
v1.0 relaunch is gated on this work being complete.

---

### API Contract Tests

**Effort:** Small

Add a set of `.cpp` files that exercise the public header surface (`moth_ui.h`, `moth_ui_fwd.h`)
purely at compile time. No assertions or test runner needed — a renamed method, removed type, or
changed signature causes a build failure. Catches accidental breakage for downstream consumers
(moth_editor, moth_graphics) before it reaches them. Only covers removal and signature changes;
semantic breakage is out of scope.

---

### v1.0 Relaunch

**Effort:** Medium (gated on prerequisite work)

The library has never been publicly published and the version number has drifted through internal
development. Once the sprite animation redesign is complete (see moth_packer TODO — this will
likely require API changes in moth_ui for per-frame pivot points and clip metadata), add the API
contract tests above, then reset the version to 1.0.0. From that point make a deliberate effort
to keep the public API stable across minor versions.
