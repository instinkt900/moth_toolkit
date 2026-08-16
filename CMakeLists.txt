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
option(MOTH_ENABLE_PHYSICS "Build the moth::physics module (Box2D rigid bodies)"      ON)
option(MOTH_ENABLE_TILEMAP "Build the moth::tilemap module (Tiled .tmj maps)"          ON)
option(MOTH_ENABLE_AUDIO   "Build the moth::audio module (miniaudio playback)"          ON)
option(MOTH_ENABLE_ASSETS  "Build the moth::assets module (id/path asset loading)"      ON)
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
if(MOTH_ENABLE_PHYSICS AND NOT MOTH_ENABLE_CORE)
    message(FATAL_ERROR "moth::physics requires moth::core — enable MOTH_ENABLE_CORE or disable MOTH_ENABLE_PHYSICS.")
endif()
if(MOTH_ENABLE_TILEMAP AND NOT (MOTH_ENABLE_CORE AND MOTH_ENABLE_GFX))
    message(FATAL_ERROR "moth::tilemap requires moth::core and moth::gfx — enable those modules or disable MOTH_ENABLE_TILEMAP.")
endif()
if(MOTH_ENABLE_AUDIO AND NOT MOTH_ENABLE_CORE)
    message(FATAL_ERROR "moth::audio requires moth::core — enable MOTH_ENABLE_CORE or disable MOTH_ENABLE_AUDIO.")
endif()
if(MOTH_ENABLE_ASSETS AND NOT MOTH_ENABLE_CORE)
    message(FATAL_ERROR "moth::assets requires moth::core — enable MOTH_ENABLE_CORE or disable MOTH_ENABLE_ASSETS.")
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
    add_subdirectory(modules/core)
endif()

if(MOTH_ENABLE_GFX)
    add_subdirectory(modules/gfx)
    # Namespaced aliases, resolved at this (top-level) scope so they are visible
    # to sibling modules (bridge, toolkit) added below. gfx defines the bare
    # target `moth_graphics`; the `moth_graphics::moth_graphics` name matches
    # what a standalone consumer gets from find_package(moth_graphics).
    add_library(moth::gfx ALIAS moth_graphics)
    add_library(moth_graphics::moth_graphics ALIAS moth_graphics)
endif()

if(MOTH_ENABLE_UI)
    add_subdirectory(modules/ui)
    add_library(moth::ui ALIAS moth_ui)
    add_library(moth_ui::moth_ui ALIAS moth_ui)
endif()

if(MOTH_ENABLE_BRIDGE)
    add_subdirectory(modules/bridge)
endif()

if(MOTH_ENABLE_ECS)
    add_subdirectory(modules/ecs)
    add_library(moth::ecs ALIAS moth_ecs)
    add_library(moth_ecs::moth_ecs ALIAS moth_ecs)
endif()

if(MOTH_ENABLE_PHYSICS)
    add_subdirectory(modules/physics)
    add_library(moth::physics ALIAS moth_physics)
    add_library(moth_physics::moth_physics ALIAS moth_physics)
endif()

if(MOTH_ENABLE_TILEMAP)
    add_subdirectory(modules/tilemap)
    add_library(moth::tilemap ALIAS moth_tilemap)
    add_library(moth_tilemap::moth_tilemap ALIAS moth_tilemap)
endif()

if(MOTH_ENABLE_AUDIO)
    add_subdirectory(modules/audio)
    add_library(moth::audio ALIAS moth_audio)
    add_library(moth_audio::moth_audio ALIAS moth_audio)
endif()

if(MOTH_ENABLE_ASSETS)
    add_subdirectory(modules/assets)
    add_library(moth::assets ALIAS moth_assets)
    add_library(moth_assets::moth_assets ALIAS moth_assets)
endif()

if(MOTH_ENABLE_TOOLKIT)
    add_subdirectory(modules/toolkit)
endif()

if(MOTH_ENABLE_EXAMPLES)
    if(MOTH_ENABLE_GFX)
        add_subdirectory(examples/sample_game)
        add_subdirectory(examples/hello_game)
    endif()
    if(MOTH_ENABLE_PHYSICS)
        add_subdirectory(examples/physics_demo)
    endif()
    if(MOTH_ENABLE_TILEMAP)
        add_subdirectory(examples/tilemap_demo)
    endif()
    if(MOTH_ENABLE_AUDIO)
        add_subdirectory(examples/audio_demo)
    endif()
    if(NOT MOTH_ENABLE_GFX AND NOT MOTH_ENABLE_PHYSICS AND NOT MOTH_ENABLE_TILEMAP AND NOT MOTH_ENABLE_AUDIO)
        message(FATAL_ERROR "MOTH_ENABLE_EXAMPLES requires at least one example-capable module (gfx, physics, tilemap, or audio).")
    endif()
endif()
