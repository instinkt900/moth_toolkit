cmake_minimum_required(VERSION 3.27)

project(moth_sample_game VERSION 0.1.0 DESCRIPTION "A minimal moth game: input, camera, scene, rotated sprites." LANGUAGES CXX)

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

if(TARGET moth::ecs)
    set(MOTH_ECS_TARGET moth::ecs)
else()
    find_package(moth_ecs CONFIG REQUIRED)
    set(MOTH_ECS_TARGET moth::ecs)
endif()

add_executable(sample_game main.cpp)
target_compile_features(sample_game PRIVATE cxx_std_17)
target_link_libraries(sample_game PRIVATE ${MOTH_GFX_TARGET} moth::core ${MOTH_ECS_TARGET})
