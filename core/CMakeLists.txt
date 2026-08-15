cmake_minimum_required(VERSION 3.27)

project(moth_core
    VERSION 0.1.0
    DESCRIPTION "Core math, platform, windowing, and event types for the Moth toolkit."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)

# GLFW provides the native windowing backend. It comes from the system on Linux
# (to match the GLFW/GTK3 stack already loaded by other tooling) and Conan on
# Windows — see conanfile.py.
if(WIN32)
    find_package(glfw3 REQUIRED)
    set(MOTH_CORE_GLFW_LINK glfw)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED IMPORTED_TARGET glfw3)
    set(MOTH_CORE_GLFW_LINK PkgConfig::GLFW3)
endif()

set(SOURCES
    src/glfw/events.cpp
    src/glfw/window.cpp
)

add_library(moth_core ${SOURCES})
add_library(moth::core ALIAS moth_core)
set_target_properties(moth_core PROPERTIES EXPORT_NAME core)

target_include_directories(moth_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_core PRIVATE src)

target_link_libraries(moth_core PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(moth_core PUBLIC ${MOTH_CORE_GLFW_LINK})

# Install + export so other modules (and external consumers) can find_package(moth_core).
# Only when built standalone; in the superbuild the modules are consumed in-tree and packaged
# individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_core EXPORT moth_coreTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
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
endif()
