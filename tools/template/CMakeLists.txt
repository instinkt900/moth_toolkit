cmake_minimum_required(VERSION 3.15)

project(@PROJECT_NAME@ VERSION 0.1.0 DESCRIPTION "A Moth game." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The packaged moth_graphics module provides the Game facade and the renderer.
find_package(moth_graphics CONFIG REQUIRED)

add_executable(@PROJECT_NAME@ main.cpp)
target_link_libraries(@PROJECT_NAME@ PRIVATE moth_graphics::moth_graphics)
