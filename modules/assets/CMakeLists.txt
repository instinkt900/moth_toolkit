cmake_minimum_required(VERSION 3.27)

project(moth_assets
    VERSION 0.1.0
    DESCRIPTION "Asset addressing (id/path) and loading for the Moth toolkit."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)

# Sibling module: in the superbuild the target already exists, standalone it is
# located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()

add_library(moth_assets
    src/assets.cpp
    src/pak.cpp)
set_target_properties(moth_assets PROPERTIES EXPORT_NAME assets)

target_include_directories(moth_assets PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_assets PRIVATE src)

target_link_libraries(moth_assets PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(moth_assets PUBLIC moth::core)

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_assets EXPORT moth_assetsTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_assetsTargets
        FILE moth_assetsTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_assets)

    configure_package_config_file(
        cmake/moth_assetsConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_assetsConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_assets)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_assetsConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_assetsConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_assetsConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_assets)
endif()
