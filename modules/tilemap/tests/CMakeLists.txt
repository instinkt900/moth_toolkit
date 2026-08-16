cmake_minimum_required(VERSION 3.15)

project(moth_tilemap_tests VERSION 1.0 DESCRIPTION "Unit tests for moth_tilemap")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 3 REQUIRED)

# Build core, gfx, and tilemap from source so the tests exercise the current
# in-tree API (the packaged moth_graphics may lag the source).
add_subdirectory(../../core moth_core)
add_subdirectory(../../gfx moth_graphics)
add_subdirectory(.. moth_tilemap)

set(SOURCES
    src/main.cpp
    src/test_tile.cpp
    src/test_tile_map.cpp
    src/test_loader.cpp
    src/test_renderer.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE moth_tilemap Catch2::Catch2)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX /utf-8)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
