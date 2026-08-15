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
# exposed to consumers via the generated features.h header.
# ---------------------------------------------------------------------------
option(MOTH_ENABLE_CORE    "Build the moth::core module (loop/platform/math/events)" ON)
option(MOTH_ENABLE_GFX     "Build the moth::gfx module (2D renderer + backends)"      ON)
option(MOTH_ENABLE_UI      "Build the moth::ui module (node graph/layers/animation)"  ON)
option(MOTH_ENABLE_BRIDGE  "Build the moth::bridge module (ui <-> gfx adapter)"       ON)
option(MOTH_ENABLE_ECS     "Build the moth::ecs module (EnTT entity-component system)" ON)
option(MOTH_ENABLE_TOOLKIT "Build the moth::toolkit aggregate target"                 ON)
option(MOTH_ENABLE_EXAMPLES "Build the example projects"                              OFF)

# ---------------------------------------------------------------------------
# Dependency constraints
# ---------------------------------------------------------------------------
if(MOTH_ENABLE_BRIDGE AND NOT (MOTH_ENABLE_CORE AND MOTH_ENABLE_GFX AND MOTH_ENABLE_UI))
    message(FATAL_ERROR "moth::bridge requires core, gfx, and ui — enable those modules or disable MOTH_ENABLE_BRIDGE.")
endif()
if(MOTH_ENABLE_GFX AND NOT MOTH_ENABLE_CORE)
    message(FATAL_ERROR "moth::gfx requires moth::core — enable MOTH_ENABLE_CORE or disable MOTH_ENABLE_GFX.")
endif()
if(MOTH_ENABLE_UI AND NOT MOTH_ENABLE_CORE)
    message(FATAL_ERROR "moth::ui requires moth::core — enable MOTH_ENABLE_CORE or disable MOTH_ENABLE_UI.")
endif()
if(MOTH_ENABLE_ECS AND NOT MOTH_ENABLE_CORE)
    message(FATAL_ERROR "moth::ecs requires moth::core — enable MOTH_ENABLE_CORE or disable MOTH_ENABLE_ECS.")
endif()

# ---------------------------------------------------------------------------
# Generated feature header (compile-time flags for consumer code):
# ---------------------------------------------------------------------------
configure_file(
    cmake/features.h.in
    ${CMAKE_BINARY_DIR}/generated/include/moth/features.h
)

# ---------------------------------------------------------------------------
# Modules
# ---------------------------------------------------------------------------
if(MOTH_ENABLE_CORE)
    add_subdirectory(core)
endif()

if(MOTH_ENABLE_GFX)
    add_subdirectory(gfx)
    # Namespaced aliases, resolved at this (top-level) scope so they are visible
    # to sibling modules (bridge, toolkit) added below. gfx defines the bare
    # target `moth_graphics`; the `moth_graphics::moth_graphics` name matches
    # what a standalone consumer gets from find_package(moth_graphics).
    add_library(moth::gfx ALIAS moth_graphics)
    add_library(moth_graphics::moth_graphics ALIAS moth_graphics)
endif()

if(MOTH_ENABLE_UI)
    add_subdirectory(ui)
    add_library(moth::ui ALIAS moth_ui)
    add_library(moth_ui::moth_ui ALIAS moth_ui)
endif()

if(MOTH_ENABLE_BRIDGE)
    add_subdirectory(bridge)
endif()

if(MOTH_ENABLE_ECS)
    add_subdirectory(ecs)
    add_library(moth::ecs ALIAS moth_ecs)
    add_library(moth_ecs::moth_ecs ALIAS moth_ecs)
endif()

if(MOTH_ENABLE_TOOLKIT)
    add_subdirectory(toolkit)
endif()

if(MOTH_ENABLE_EXAMPLES)
    if(NOT MOTH_ENABLE_GFX)
        message(FATAL_ERROR "MOTH_ENABLE_EXAMPLES requires MOTH_ENABLE_GFX.")
    endif()
    add_subdirectory(examples/sample_game)
endif()
