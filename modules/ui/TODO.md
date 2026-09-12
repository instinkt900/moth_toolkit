# TODO

## 1.x

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
