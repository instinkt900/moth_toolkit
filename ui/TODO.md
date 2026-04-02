# TODO

- **Doxygen API docs** — add a Doxyfile targeting `include/moth_ui`, configure GitHub Actions to generate and publish to GitHub Pages on push to master. Consider Doxygen Awesome for a modern HTML theme.

- **C++ standard upgrade** — evaluate moving from C++17 to C++20 or C++23. C++20 brings concepts, ranges, `std::span`, coroutines, and `std::format` (replacing fmt); C++23 adds `std::expected`, `std::mdspan`, and further ranges improvements. Requires updating CMakeLists.txt, Conan profiles, and verifying compiler support on the CI matrix (MSVC, GCC, Clang).



- **Review `IRenderer::RenderImage` mutability requirement** — `RenderImage` takes `IImage&` (non-const), which forces `IFlipbook::GetImage()` to also return a non-const ref (otherwise the call site won't compile). It is unlikely a renderer genuinely needs to mutate an image while drawing it — this is probably an oversight from when the interface was first written. Audit all concrete `RenderImage` implementations to confirm whether the non-const ref is actually used for mutation, and if not, change the signature to `IImage const&` throughout. That would also allow `IFlipbook::GetImage()` to correctly return `IImage const&`, closing the const-correctness gap flagged in the review.

- **Revisit factory ownership model** — `IImageFactory` and `IFlipbookFactory` return `unique_ptr`, implying the caller owns the resource, yet both expose `FlushCache()`, implying the factory also holds onto something. These two things contradict each other. Decide on one model: either factories return `shared_ptr` and cache internally (factory co-owns, `FlushCache` makes sense), or factories return `unique_ptr` and hold no state (`FlushCache` is removed). The choice will also affect whether multiple nodes using the same asset path share a single instance or each hold their own copy.

- **Logging interface** — add an optional `ILogger` interface (similar to `IFlipbookFactory`) that the application can implement and register with `Context`. moth_ui calls through it for internal errors and warnings (bad assets, load failures, etc.); if none is registered messages are silently dropped. This avoids pulling spdlog or any other logging library into moth_ui directly. First callsite: the `SheetDesc` validation failure in `NodeFlipbook::Load()`.

- **Flipbook animation** — animate sprite sheet frames on `NodeImage` via the existing keyframe system. Steps: (1) add a virtual `GetAnimationBindings()` to `Node` so subclasses can expose extra float targets to `AnimationController` (the main architectural hook); (2) add `FrameIndex` to `AnimationTrack::Target`; (3) add `m_frames` (`std::vector<IntRect>`) to `LayoutEntityImage`/`NodeImage` — on each tick `int(m_currentFrame)` indexes into it to set `m_sourceRect`; (4) serialize `m_frames` in `LayoutEntityImage`. Editor support (frame rect authoring, FrameIndex keyframing in the timeline) is a separate effort on the moth_editor side.
