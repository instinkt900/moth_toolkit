cmake_minimum_required(VERSION 3.15)

project(moth_hello_game VERSION 0.1.0 DESCRIPTION "The minimal moth game: a window, a scene, and a moving square." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# In-tree (superbuild): the module target already exists.
# Standalone: locate the packaged module via find_package.
if(NOT TARGET moth::gfx)
    find_package(moth_graphics CONFIG REQUIRED)
    set(MOTH_GFX_TARGET moth_graphics::moth_graphics)
else()
    set(MOTH_GFX_TARGET moth::gfx)
endif()

add_executable(hello_game main.cpp)
target_link_libraries(hello_game PRIVATE ${MOTH_GFX_TARGET})
