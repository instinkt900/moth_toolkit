cmake_minimum_required(VERSION 3.27)

project(moth_ecs
    VERSION 0.1.0
    DESCRIPTION "A lightweight EnTT-backed entity-component system for the Moth toolkit."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(EnTT CONFIG REQUIRED)

# Sibling module: in the superbuild the target already exists, standalone it is
# located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()

add_library(moth_ecs INTERFACE)
set_target_properties(moth_ecs PROPERTIES EXPORT_NAME ecs)

target_include_directories(moth_ecs INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(moth_ecs INTERFACE EnTT::EnTT)
target_link_libraries(moth_ecs INTERFACE moth::core)

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_ecs EXPORT moth_ecsTargets)
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_ecsTargets
        FILE moth_ecsTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_ecs)

    configure_package_config_file(
        cmake/moth_ecsConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_ecsConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_ecs)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_ecsConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_ecsConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_ecsConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_ecs)
endif()
