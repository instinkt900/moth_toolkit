cmake_minimum_required(VERSION 3.15)

project(moth_sample_game VERSION 0.1.0 DESCRIPTION "A minimal moth game: input, camera, scene, rotated sprites." LANGUAGES CXX)

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

add_executable(sample_game main.cpp)
target_link_libraries(sample_game PRIVATE ${MOTH_GFX_TARGET} moth::core)
