cmake_minimum_required(VERSION 3.15)

project(moth_tilemap_demo VERSION 0.1.0 DESCRIPTION "A moth::tilemap demo: loads a TMJ map and renders it under a pannable camera." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# In-tree (superbuild): the module target already exists.
# Standalone: locate the packaged module via find_package.
if(NOT TARGET moth::tilemap)
    find_package(moth_tilemap CONFIG REQUIRED)
endif()

add_executable(tilemap_demo main.cpp)
target_link_libraries(tilemap_demo PRIVATE moth::tilemap)
