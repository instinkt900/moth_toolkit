cmake_minimum_required(VERSION 3.27)

project(moth_anim_tests VERSION 1.0 DESCRIPTION "Unit tests for moth_anim")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 3 REQUIRED)

# Build core, gfx, and anim from source so the tests exercise the current
# in-tree API (the packaged moth_graphics may lag the source).
add_subdirectory(../../core moth_core)
add_subdirectory(../../gfx moth_graphics)
add_subdirectory(.. moth_anim)

set(SOURCES
    src/main.cpp
    src/test_anim_set.cpp
    src/test_animator.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE moth_anim Catch2::Catch2)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX /utf-8)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
