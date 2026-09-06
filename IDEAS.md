# Ideas

A loose list of features and improvements worth considering at some point.
Nothing here is planned or promised — treat it as scratch notes, not a roadmap.

## Streaming textures

Add a non-blocking texture-upload path for streaming large worlds (e.g. a
procedural heightmap or a tiled background paged in on demand).

Today every upload path blocks on the GPU: `TextureFromPixels`/`TextureFromMemory`
(through `Texture::FromRGBA`) and `ITexture::UpdatePixels` all end in a
`SubmitAndWait`. The pieces for an async version already exist — `Fence` and
`CommandBuffer::Submit(fence, ...)` — so the work is mostly plumbing:

- `UploadPixelsAsync` (or similar) that records the copy, submits with a fence,
  and returns a pollable handle instead of blocking.
- A persistent staging buffer reused across uploads (both current paths allocate
  a fresh staging `Buffer` per call).
- The handle must own the staging buffer until the fence signals, and the texture
  stays unsampled until then (the natural placeholder window).
- Single graphics queue means "async" here is non-blocking on the render thread,
  not upload from a worker thread.
