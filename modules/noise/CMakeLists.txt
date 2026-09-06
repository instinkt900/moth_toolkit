cmake_minimum_required(VERSION 3.27)

project(moth_noise
    VERSION 0.1.0
    DESCRIPTION "Node-graph noise generation (FastNoise2) for the Moth toolkit."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)
find_package(FastNoise2 REQUIRED)

# Sibling module: in the superbuild the target already exists, standalone it is
# located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()

add_library(moth_noise
    src/node_tree.cpp)
set_target_properties(moth_noise PROPERTIES EXPORT_NAME noise)

if(MSVC)
    target_compile_options(moth_noise PRIVATE /W4)
else()
    target_compile_options(moth_noise PRIVATE -Wall -Wextra)
endif()

target_include_directories(moth_noise PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_noise PRIVATE src)

target_link_libraries(moth_noise PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(moth_noise PUBLIC FastNoise2::FastNoise)
target_link_libraries(moth_noise PUBLIC moth::core)

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_noise EXPORT moth_noiseTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_noiseTargets
        FILE moth_noiseTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_noise)

    configure_package_config_file(
        cmake/moth_noiseConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_noiseConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_noise)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_noiseConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_noiseConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_noiseConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_noise)
endif()
