cmake_minimum_required(VERSION 3.27)

project(moth_physics
    VERSION 0.1.0
    DESCRIPTION "A Box2D-backed 2D physics module for the Moth toolkit."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(box2d CONFIG REQUIRED)

# Sibling module: in the superbuild the target already exists, standalone it is
# located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()

add_library(moth_physics src/world.cpp)
set_target_properties(moth_physics PROPERTIES EXPORT_NAME physics)

target_include_directories(moth_physics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_physics PRIVATE src)

target_link_libraries(moth_physics PUBLIC box2d::box2d)
target_link_libraries(moth_physics PUBLIC moth::core)

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_physics EXPORT moth_physicsTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_physicsTargets
        FILE moth_physicsTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_physics)

    configure_package_config_file(
        cmake/moth_physicsConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_physicsConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_physics)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_physicsConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_physicsConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_physicsConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_physics)
endif()
