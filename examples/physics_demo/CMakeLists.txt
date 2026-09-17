cmake_minimum_required(VERSION 3.27)

project(moth_physics_demo VERSION 0.1.0 DESCRIPTION "A headless moth::physics body-drop demo." LANGUAGES CXX)

# In-tree (superbuild): the module target already exists.
# Standalone: locate the packaged module via find_package.
if(NOT TARGET moth::physics)
    find_package(moth_physics CONFIG REQUIRED)
endif()

add_executable(physics_demo main.cpp)
target_compile_features(physics_demo PRIVATE cxx_std_17)
target_link_libraries(physics_demo PRIVATE moth::physics)
