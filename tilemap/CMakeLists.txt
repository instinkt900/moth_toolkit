cmake_minimum_required(VERSION 3.27)

project(moth_tilemap
    VERSION 0.1.0
    DESCRIPTION "Grid-based tilemaps and tilesets (Tiled .tmj import + rendering) for the Moth toolkit."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)

# Sibling modules: in the superbuild the targets already exist, standalone they
# are located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()
if(NOT TARGET moth_graphics)
    find_package(moth_graphics CONFIG REQUIRED)
endif()

# Resolve the gfx link target: the namespaced package name (standalone / Conan),
# the superbuild alias, or the bare in-tree target (source-built tests).
if(TARGET moth_graphics::moth_graphics)
    set(MOTH_TILEMAP_GFX moth_graphics::moth_graphics)
elseif(TARGET moth::gfx)
    set(MOTH_TILEMAP_GFX moth::gfx)
else()
    set(MOTH_TILEMAP_GFX moth_graphics)
endif()

add_library(moth_tilemap
    src/tile_map_loader.cpp
    src/tile_map_renderer.cpp)
set_target_properties(moth_tilemap PROPERTIES EXPORT_NAME tilemap)

target_include_directories(moth_tilemap PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_tilemap PRIVATE src)

target_link_libraries(moth_tilemap PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(moth_tilemap PUBLIC moth::core)
target_link_libraries(moth_tilemap PUBLIC ${MOTH_TILEMAP_GFX})

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_tilemap EXPORT moth_tilemapTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_tilemapTargets
        FILE moth_tilemapTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_tilemap)

    configure_package_config_file(
        cmake/moth_tilemapConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_tilemapConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_tilemap)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_tilemapConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_tilemapConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_tilemapConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_tilemap)
endif()
