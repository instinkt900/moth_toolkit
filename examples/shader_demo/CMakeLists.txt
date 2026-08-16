cmake_minimum_required(VERSION 3.15)

project(moth_shader_demo VERSION 0.1.0 DESCRIPTION "Compiles and draws a Shadertoy-style shader." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# In-tree (superbuild): the module targets already exist.
# Standalone: locate the packaged modules via find_package.
if(TARGET moth::gfx)
    set(MOTH_GFX_TARGET moth::gfx)
else()
    find_package(moth_graphics CONFIG REQUIRED)
    set(MOTH_GFX_TARGET moth_graphics::moth_graphics)
endif()

if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()

add_executable(shader_demo main.cpp)
target_link_libraries(shader_demo PRIVATE ${MOTH_GFX_TARGET} moth::core)
