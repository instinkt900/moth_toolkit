cmake_minimum_required(VERSION 3.15)

project(moth_audio_demo VERSION 0.1.0 DESCRIPTION "A moth::audio demo: plays a one-shot sound and looping music." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# In-tree (superbuild): the module target already exists.
# Standalone: locate the packaged module via find_package.
if(NOT TARGET moth::audio)
    find_package(moth_audio CONFIG REQUIRED)
endif()

add_executable(audio_demo main.cpp)
target_link_libraries(audio_demo PRIVATE moth::audio)
