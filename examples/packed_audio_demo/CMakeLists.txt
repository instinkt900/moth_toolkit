cmake_minimum_required(VERSION 3.15)

project(moth_packed_audio_demo VERSION 0.1.0 DESCRIPTION "Cooks a WAV into a .pak and plays it back by id." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# In-tree (superbuild): the module targets already exist.
# Standalone: locate the packaged modules via find_package.
if(NOT TARGET moth::audio)
    find_package(moth_audio CONFIG REQUIRED)
endif()
if(NOT TARGET moth::assets)
    find_package(moth_assets CONFIG REQUIRED)
endif()

add_executable(packed_audio_demo main.cpp)
target_link_libraries(packed_audio_demo PRIVATE moth::audio moth::assets)
