cmake_minimum_required(VERSION 3.27)

project(moth_pak VERSION 0.1.0 DESCRIPTION "Cooks a folder of assets into a .pak + manifest.json." LANGUAGES CXX)

# In-tree (superbuild): the module target already exists.
# Standalone: locate the packaged module via find_package.
if(NOT TARGET moth::assets)
    find_package(moth_assets CONFIG REQUIRED)
endif()

add_executable(moth_pak main.cpp)
target_compile_features(moth_pak PRIVATE cxx_std_17)
target_link_libraries(moth_pak PRIVATE moth::assets)
