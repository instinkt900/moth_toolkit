cmake_minimum_required(VERSION 3.27)

project(moth_core
    VERSION 0.1.0
    DESCRIPTION "Core math, platform, and event types for the Moth toolkit."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)

# Header-only for now (math/types); platform/loop .cpp files land in a later step.
add_library(moth_core INTERFACE)
add_library(moth::core ALIAS moth_core)
set_target_properties(moth_core PROPERTIES EXPORT_NAME core)

target_include_directories(moth_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(moth_core INTERFACE nlohmann_json::nlohmann_json)

# Install + export so other modules (and external consumers) can find_package(moth_core).
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(TARGETS moth_core EXPORT moth_coreTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT moth_coreTargets
    FILE moth_coreTargets.cmake
    NAMESPACE moth::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_core)

configure_package_config_file(
    cmake/moth_coreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/moth_coreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_core)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/moth_coreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/moth_coreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/moth_coreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_core)
