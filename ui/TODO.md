# TODO

## 1.x

### Switch Rotation Storage to Radians

**Effort:** Medium

Rotation is currently in **degrees** end-to-end (`Node::m_rotation`, `Node::SetRotation/GetRotation`,
`LayoutEntity::GetRotationAtFrame`, `FloatMat4x4::Rotation`, the `AnimationTrack::Target::Rotation`
keyframe values, and the `.mothui` file format). The newly added gradient angle stores radians,
which makes the codebase inconsistent. The desired end state is radians at every API level; the
editor converts to degrees for display only (same pattern used by the gradient angle field).

**Scope:**
- Flip `m_rotation` / `SetRotation` / `GetRotation` / `GetRotationAtFrame` semantics to radians.
- Drop the internal `*= kDegToRad` in `FloatMat4x4::Rotation`; rename `GetRotationDegrees`
  to `GetRotationRadians`.
- Editor `editor_panel_properties.cpp` Rotation input + `editor_layer.cpp` rotation-edit
  context: convert deg ↔ rad at the UI boundary.
- Bounds handles (`rotation_bounds_handle.cpp`, `pivot_bounds_handle.cpp`,
  `offset_bounds_handle.cpp`) currently do `GetRotation() * kDegToRad`; just remove the multiply.
- `editor_panel_canvas.cpp` `RotateAroundPivot(..., -node.GetRotation())` callers — either
  the helper takes radians or the caller converts.
- Test updates in `test_node.cpp` (`SetRotation(45.0f)` etc.) and `test_transform.cpp`
  (`FloatMat4x4::Rotation(90.0f, ...)` round-trips).

**File-format migration:** `.mothui` files store the `Rotation` track's keyframe values as raw
floats (currently degrees). A version bump + load-time `*= kDegToRad` for old files is needed
or every existing layout with a Rotation track will render ~57× over-rotated. Until this
migration is in place, the refactor stays deferred.

---

### Rotated Clip Regions (NodeClip + NodeGradient)

**Effort:** Large

NodeClip and NodeGradient currently rely on backend scissor (`SDL_RenderSetClipRect`,
`vkCmdSetScissor`), which is axis-aligned screen-space only. As a result:

- **NodeClip cannot rotate.** A NodeClip's clip region is computed as the AABB of its
  bounds — fine when the node is axis-aligned, but the moment a clip is rotated the
  intended mask becomes the AABB, not the rotated quad.
- **NodeGradient cannot rotate.** The MothRenderer wrapper pushes an AABB scissor sized
  to the transformed node corners. For a non-rotated node it matches the rect exactly;
  for a rotated node the AABB is larger than the visible quad, so the gradient leaks
  past the rotated edges. (A leftover `PushClip(screenAabb)` hack lives in
  `moth_graphics/.../moth_renderer.cpp` `RenderGradientRect` for this reason.)

**The fix** would route clipping through a polygon stack (`IGraphics::PushClipPolygon`),
honoured per-backend:

- **SDL backend**: CPU-side Sutherland-Hodgman chop of every emitted primitive's geometry
  against the active polygon before submission to `SDL_RenderGeometry`. Textured
  primitives (`DrawImage`, `DrawImageTiled`, text, nine-slice) additionally need UV
  interpolation at cut vertices. Nested polygons compose by intersection.
- **Vulkan backend**: stencil buffer (draw rotated quad with increment, stencil-test
  contents, decrement on pop) or fragment-shader half-plane discard with the four edge
  equations passed as uniforms.

The work is substantial and touches every primitive draw path. Until then, **clip and
gradient nodes should be treated as non-rotatable** — the editor still permits setting
rotation on them but the visual result will be wrong (gradient leaks past rotated edges;
clip masks revert to AABB of the rotated bounds).

---

### NodePainter — CPU "fragment shader" node

**Effort:** Medium

Now that `IGraphics::UpdateTexture` lets us push pixel buffers per frame, we can run
arbitrary per-pixel fill functions on the CPU and treat the result as a texture — a
backend-agnostic substitute for fragment shaders. Use cases: plasma/fire explosions,
energy fields, animated dithers, noise washes, palette-cycled effects. Footprints stay
small (typically 64²–128² stretched onto the node rect), so cost stays well under a
frame budget even with several instances on screen.

**Sketch:**

- New `Node` subclass `NodePainter` with a swappable `IFillFunction`:
  ```cpp
  struct IFillFunction {
      virtual void Fill(uint32_t* pixels, int w, int h, float t, void* params) = 0;
  };
  ```
- Owns an `IImage` sized to a configurable internal resolution (independent of node
  rect size); rebuilds when resolution changes.
- `DrawInternal` calls `fillFn->Fill(...)`, uploads via `UpdateTexture`, then draws
  the image stretched into the node bounds.
- Animatable scalars exposed as `AnimationTarget` entries for `t` and a few generic
  `paramN` floats, so the fill can be keyframed in the editor.

**Design notes:**

- Keep the fill function a **plain C++ function over a pixel buffer** — same shape as
  a GLSL/SPIR-V fragment shader. That way a Vulkan-only GPU path can later compile
  the same logic into a real shader without changing call sites.
- One internal texture **per painter instance**, sized to the effect; do not share a
  scratch buffer. Lifetime-bound to the node.
- Palette-LUT pattern is the cheap default: compute greyscale intensity, index a
  `Color[256]`, cycle the offset for colour-cycling effects.
- Streaming-access texture path: confirm `UpdateTexture` routes through
  `SDL_TEXTUREACCESS_STREAMING` lock/unlock (not a copy onto a static texture) for the
  hot per-frame case; if not, add a streaming variant.
- Sample fills shipped with moth_ui (gradient is already a node; plasma, dither,
  scanlines could ship as reference `IFillFunction`s) or leave entirely to consumers.
