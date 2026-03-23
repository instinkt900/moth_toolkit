# TODO

- **Doxygen API docs** — add a Doxyfile targeting `include/moth_ui`, configure GitHub Actions to generate and publish to GitHub Pages on push to master. Consider Doxygen Awesome for a modern HTML theme.

- **C++ standard upgrade** — evaluate moving from C++17 to C++20 or C++23. C++20 brings concepts, ranges, `std::span`, coroutines, and `std::format` (replacing fmt); C++23 adds `std::expected`, `std::mdspan`, and further ranges improvements. Requires updating CMakeLists.txt, Conan profiles, and verifying compiler support on the CI matrix (MSVC, GCC, Clang).



- **Flipbook animation** — animate sprite sheet frames on `NodeImage` via the existing keyframe system. Steps: (1) add a virtual `GetAnimationBindings()` to `Node` so subclasses can expose extra float targets to `AnimationController` (the main architectural hook); (2) add `FrameIndex` to `AnimationTrack::Target`; (3) add `m_frames` (`std::vector<IntRect>`) to `LayoutEntityImage`/`NodeImage` — on each tick `int(m_currentFrame)` indexes into it to set `m_sourceRect`; (4) serialize `m_frames` in `LayoutEntityImage`. Editor support (frame rect authoring, FrameIndex keyframing in the timeline) is a separate effort on the moth_editor side.
