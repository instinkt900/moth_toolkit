cmake_minimum_required(VERSION 3.15)

project(moth_graphics_tests VERSION 1.0 DESCRIPTION "Unit tests for moth_graphics")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 3 REQUIRED)

add_subdirectory(.. moth_graphics)

set(SOURCES
    src/main.cpp
    src/test_spritesheet.cpp
    src/test_sprite.cpp
    src/test_shader.cpp
    src/test_ticker.cpp
    src/test_event_emitter.cpp
    src/test_polygon_triangulation.cpp
    src/test_camera.cpp
    # API surface tests — compile-time only, no runtime assertions
    src/api_surface_umbrella.cpp
    src/api_surface_enums.cpp
    src/api_surface_graphics.cpp
    src/api_surface_sprite.cpp
    src/api_surface_events.cpp
    src/api_surface_platform.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
# The glsl compiler lives in moth_graphics' private src tree; the GLSL tests
# include it directly.
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(${PROJECT_NAME} PRIVATE moth_graphics Catch2::Catch2)

# Mirror moth_graphics' glslang toggle so the GLSL tests compile with/without it.
if(MOTH_GRAPHICS_ENABLE_GLSLANG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MOTH_GRAPHICS_ENABLE_GLSLANG=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE MOTH_GRAPHICS_ENABLE_GLSLANG=0)
endif()

if(MSVC)
    # /utf-8: interpret source files as UTF-8 so Unicode chars in headers (e.g. moth_ui_format.h) don't trigger C4566.
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX /utf-8)
    target_link_options(${PROJECT_NAME} PRIVATE /ignore:4099)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
