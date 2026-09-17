cmake_minimum_required(VERSION 3.27)

project(moth_profile_tests VERSION 1.0 DESCRIPTION "Unit tests for moth_profile")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 3 REQUIRED)
find_package(Threads REQUIRED)

# Build the recorder only; the ImGui panel would pull in moth_graphics.
set(MOTH_PROFILE_ENABLE_IMGUI OFF)
add_subdirectory(.. moth_profile)

set(SOURCES
    src/main.cpp
    src/test_profiler.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE moth_profile Catch2::Catch2 Threads::Threads)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX /utf-8)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
