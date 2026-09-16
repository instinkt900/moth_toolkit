# Build-context tool overrides for Windows CI.
#
# ConanCenter recipes tool_require CMake 3.31, which predates the
# "Visual Studio 18 2026" generator that Conan selects for MSVC 195. The runner
# image (windows-2025-vs2026) ships Visual Studio 2026, so dependencies such as
# vulkan-loader fail to configure with:
#
#   CMake Error: Could not create named generator Visual Studio 18 2026
#
# The runner's own CMake handles that generator -- the superbuild configures
# with it fine -- so only the CMake that recipes pull in needs replacing. A
# profile [tool_requires] overrides the recipes' own, for every package.
#
# Composed after the default build profile:
#   conan install . -pr:b default -pr:b .github/conan/windows-tools.profile
#
# Tool requirements are build-context only, so this does not affect any
# package_id: it changes what builds the dependencies, not what is built.
[tool_requires]
cmake/4.4.3
