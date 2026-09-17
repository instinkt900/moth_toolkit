cmake_minimum_required(VERSION 3.27)

project(moth_core
    VERSION 0.1.0
    DESCRIPTION "Core math, platform, windowing, and event types for the Moth toolkit."
    LANGUAGES CXX)

# The platform layer is the only part of this module that needs GLFW. Consumers
# that want the math/event types without a windowing stack (headless tools, an
# external editor that already owns its own window) can turn it off. The
# backend-agnostic halves — window.h, input.h/input.cpp — are always built; only
# the GLFW implementation under */glfw/ is gated.
option(MOTH_CORE_ENABLE_PLATFORM "Build the GLFW windowing/event backend" ON)

find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

# GLFW provides the native windowing backend. It comes from the system on Linux
# (to match the GLFW/GTK3 stack already loaded by other tooling) and Conan on
# Windows — see conanfile.py.
if(MOTH_CORE_ENABLE_PLATFORM)
    if(WIN32)
        find_package(glfw3 REQUIRED)
        set(MOTH_CORE_GLFW_LINK glfw)
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GLFW3 REQUIRED IMPORTED_TARGET glfw3)
        set(MOTH_CORE_GLFW_LINK PkgConfig::GLFW3)
    endif()
endif()

set(SOURCES
    src/input.cpp
    src/log.cpp
)

if(MOTH_CORE_ENABLE_PLATFORM)
    list(APPEND SOURCES
        src/glfw/events.cpp
        src/glfw/gamepad.cpp
        src/glfw/window.cpp
    )
endif()

add_library(moth_core ${SOURCES})
target_compile_features(moth_core PUBLIC cxx_std_17)
add_library(moth::core ALIAS moth_core)
set_target_properties(moth_core PROPERTIES EXPORT_NAME core)

if(MSVC)
    target_compile_options(moth_core PRIVATE /W4)
else()
    target_compile_options(moth_core PRIVATE -Wall -Wextra)
endif()

# clang-tidy is opt-in via MOTH_ENABLE_CLANG_TIDY, matching moth_ui and
# moth_graphics. core is linted here rather than relying on a consumer to do it:
# its headers used to be checked only incidentally, by whichever module included
# them, under that module's ruleset -- so a magic number in transform.h broke
# moth_ui's build and gave core's author no signal at all.
#
# clang-tidy reads a clang command line, so it cannot lint an MSVC build.
option(MOTH_ENABLE_CLANG_TIDY "Run clang-tidy over moth_core's sources" OFF)
if(MOTH_ENABLE_CLANG_TIDY AND NOT MSVC)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
    set_target_properties(moth_core PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

target_include_directories(moth_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_core PRIVATE src)

target_link_libraries(moth_core PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(moth_core PUBLIC fmt::fmt)
target_link_libraries(moth_core PUBLIC spdlog::spdlog)
if(MOTH_CORE_ENABLE_PLATFORM)
    target_link_libraries(moth_core PUBLIC ${MOTH_CORE_GLFW_LINK})
endif()

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
    if(MOTH_CORE_ENABLE_PLATFORM)
        install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    else()
        install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            PATTERN "glfw" EXCLUDE)
    endif()
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
