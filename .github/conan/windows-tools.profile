# Build-context tool overrides for Windows CI.
#
# ConanCenter recipes tool_require CMake with ranges that predate the
# "Visual Studio 18 2026" generator Conan selects for MSVC 195 -- vulkan-loader
# asks for cmake/[>=3.17.2 <4.0], which resolves to 3.31.12 and fails with:
#
#   CMake Error: Could not create named generator Visual Studio 18 2026
#
# The generator itself is fine: the runner's own CMake drives it, and the
# superbuild configures and builds with it. Only the CMake the recipes bring
# along is too old.
#
# This has to be replace_tool_requires rather than tool_requires. A profile
# [tool_requires] is added to the graph but does not displace a recipe's own
# range: both CMakes end up in the graph and the recipe still builds with its
# 3.31. replace_tool_requires rewrites the reference itself, range and all.
#
# Composed after the default build profile:
#   conan install . -pr:b default -pr:b .github/conan/windows-tools.profile
#
# Tool requirements are build-context only and do not enter any package_id, so
# this changes what builds the dependencies, not what is built.
[replace_tool_requires]
cmake/*: cmake/4.4.3
