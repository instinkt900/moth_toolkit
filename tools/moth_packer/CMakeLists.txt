cmake_minimum_required(VERSION 3.27)

project(moth_packer_cli
    VERSION 1.0.0
    DESCRIPTION "Packs images into texture atlases/flipbooks and extracts sprites from sheets."
    LANGUAGES CXX)

find_package(CLI11 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

# In-tree (superbuild): the module target already exists.
# Standalone: locate the packaged module via find_package.
if(NOT TARGET moth::packer)
    find_package(moth_packer CONFIG REQUIRED)
endif()

add_executable(moth_packer_cli main.cpp)
target_compile_features(moth_packer_cli PRIVATE cxx_std_17)
# The library target is `moth_packer`; the binary takes that name too.
set_target_properties(moth_packer_cli PROPERTIES OUTPUT_NAME moth_packer)
target_compile_definitions(moth_packer_cli PRIVATE MOTH_PACKER_VERSION="${PROJECT_VERSION}")
target_link_libraries(moth_packer_cli PRIVATE
    moth::packer
    CLI11::CLI11
    fmt::fmt
    spdlog::spdlog)

if(MSVC)
    target_compile_definitions(moth_packer_cli PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(moth_packer_cli PRIVATE /W3 /WX)
else()
    target_compile_options(moth_packer_cli PRIVATE -Wall -Werror)
endif()

include(GNUInstallDirs)
install(TARGETS moth_packer_cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
