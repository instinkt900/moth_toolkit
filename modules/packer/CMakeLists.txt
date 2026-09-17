cmake_minimum_required(VERSION 3.27)

project(moth_packer
    VERSION 1.0.0
    DESCRIPTION "Texture atlas/flipbook packing and sprite-sheet extraction for the Moth toolkit."
    LANGUAGES CXX C)

# The layout collectors (CollectImagesFromLayout/CollectImagesFromLayoutsDir) are
# the only part of this module that needs moth::ui. They are opt-in so that a
# consumer which only packs images — moth_sprite, for instance — links nothing
# beyond moth::core.
option(MOTH_PACKER_ENABLE_UI "Build the moth::ui layout image collectors" OFF)

find_package(stb REQUIRED)
find_package(p-ranav-glob REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(range-v3 REQUIRED)
find_package(spdlog REQUIRED)

# Sibling module: in the superbuild the target already exists, standalone it is
# located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()
if(MOTH_PACKER_ENABLE_UI AND NOT TARGET moth::ui)
    find_package(moth_ui CONFIG REQUIRED)
    # Unlike moth_core, the moth_ui recipe does not set a cmake_target_name, so
    # the package provides moth_ui::moth_ui. Alias it to the name the superbuild
    # uses so the link line below is the same either way.
    if(NOT TARGET moth::ui)
        add_library(moth::ui ALIAS moth_ui::moth_ui)
    endif()
endif()

add_library(moth_packer
    src/packer.cpp
    src/stb_impl.c)
target_compile_features(moth_packer PUBLIC cxx_std_17)
set_target_properties(moth_packer PROPERTIES EXPORT_NAME packer)

target_include_directories(moth_packer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_packer PRIVATE src)

# Only the math types (IntVec2/IntRect) reach the public header, so moth::core is
# the module's sole public dependency; the rest are implementation details.
target_link_libraries(moth_packer PUBLIC moth::core)
target_link_libraries(moth_packer PRIVATE
    stb::stb
    p-ranav-glob::p-ranav-glob
    nlohmann_json::nlohmann_json
    range-v3::range-v3
    spdlog::spdlog)

if(MOTH_PACKER_ENABLE_UI)
    # PUBLIC: the guard also gates the declarations in packer.h, so consumers
    # must see the same definition the library was built with.
    target_compile_definitions(moth_packer PUBLIC MOTH_PACKER_HAS_UI)
    target_link_libraries(moth_packer PRIVATE moth::ui)
endif()

if(MSVC)
    target_compile_definitions(moth_packer PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(moth_packer PRIVATE /W3 /WX)
else()
    target_compile_options(moth_packer PRIVATE -Wall -Werror)
endif()

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_packer EXPORT moth_packerTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_packerTargets
        FILE moth_packerTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_packer)

    configure_package_config_file(
        cmake/moth_packerConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_packerConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_packer)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_packerConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_packerConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_packerConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_packer)
endif()
