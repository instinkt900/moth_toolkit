cmake_minimum_required(VERSION 3.15)

project(moth_packer VERSION 0.1.0 DESCRIPTION "Cooks a folder of assets into a .pak + manifest.json." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# In-tree (superbuild): the module target already exists.
# Standalone: locate the packaged module via find_package.
if(NOT TARGET moth::assets)
    find_package(moth_assets CONFIG REQUIRED)
endif()

add_executable(moth_packer main.cpp)
target_link_libraries(moth_packer PRIVATE moth::assets)
