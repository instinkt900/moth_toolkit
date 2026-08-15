cmake_minimum_required(VERSION 3.27)

project(moth_toolkit
    VERSION 0.1.0
    DESCRIPTION "A modular, code-first 2D game engine toolkit."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Module toggles
#
# Each module is a standalone library under the `moth::` namespace, consumable
# on its own (find_package(moth_gfx)) or as part of this toolkit. Turning a
# module off removes it (and its dependencies) from the build; the flag is
# exposed to consumers via target_compile_definitions(... PUBLIC ...) once the
# modules are wired in.
# ---------------------------------------------------------------------------
option(MOTH_ENABLE_CORE    "Build the moth::core module (loop/platform/math/events)" ON)
option(MOTH_ENABLE_GFX     "Build the moth::gfx module (2D renderer + backends)"      ON)
option(MOTH_ENABLE_UI      "Build the moth::ui module (node graph/layers/animation)"  ON)
option(MOTH_ENABLE_BRIDGE  "Build the moth::bridge module (ui <-> gfx adapter)"       OFF)
option(MOTH_ENABLE_TOOLKIT "Build the moth::toolkit aggregate target"                 ON)

# ---------------------------------------------------------------------------
# Status / roadmap
#
# gfx/ and ui/ are imported from their original repos (roadmap Phase 3) and
# remain standalone-able; their own CMakeLists are not yet consumed by this
# superbuild. core/ and bridge/ are placeholders pending the core extraction
# (Phase 1) and the ui<->gfx bridge split (Phase 2).
#
# TODO(Phase 1): extract moth::core from gfx/ (platform/, Ticker, events, math),
#                then replace the core/ placeholder with the real library.
# TODO(Phase 2): move the moth_ui bridge out of gfx/ into bridge/ and gate ui
#                behind MOTH_ENABLE_UI.
# TODO(Phase 4): add_subdirectory(gfx) / add_subdirectory(ui) once they consume
#                moth::core and no longer hard-depend on moth_ui types.
# ---------------------------------------------------------------------------

# Generated feature header (compile-time flags for consumer code):
configure_file(
    cmake/features.h.in
    ${CMAKE_BINARY_DIR}/generated/include/moth/features.h
)

if(MOTH_ENABLE_CORE)
    add_subdirectory(core)
endif()

if(MOTH_ENABLE_BRIDGE)
    add_subdirectory(bridge)
endif()

if(MOTH_ENABLE_TOOLKIT)
    add_subdirectory(toolkit)
endif()
