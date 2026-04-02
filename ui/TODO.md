# TODO

- **C++ standard upgrade** — evaluate moving from C++17 to C++20 or C++23. C++20 brings concepts, ranges, `std::span`, coroutines, and `std::format` (replacing fmt); C++23 adds `std::expected`, `std::mdspan`, and further ranges improvements. Requires updating CMakeLists.txt, Conan profiles, and verifying compiler support on the CI matrix (MSVC, GCC, Clang).
