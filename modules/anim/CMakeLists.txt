cmake_minimum_required(VERSION 3.27)

project(moth_anim
    VERSION 0.1.0
    DESCRIPTION "Data-driven character animation state machine for the Moth toolkit."
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
    set(MOTH_ANIM_GFX moth_graphics::moth_graphics)
elseif(TARGET moth::gfx)
    set(MOTH_ANIM_GFX moth::gfx)
else()
    set(MOTH_ANIM_GFX moth_graphics)
endif()

add_library(moth_anim
    src/anim_set.cpp
    src/animator.cpp
    src/animator_factory.cpp)
set_target_properties(moth_anim PROPERTIES EXPORT_NAME anim)

target_include_directories(moth_anim PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_anim PRIVATE src)

# nlohmann_json appears only in the .cpp files (the public headers expose no JSON
# types), so it stays private.
target_link_libraries(moth_anim PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(moth_anim PUBLIC moth::core)
target_link_libraries(moth_anim PUBLIC ${MOTH_ANIM_GFX})

if(MSVC)
    target_compile_options(moth_anim PRIVATE /W3 /WX)
else()
    target_compile_options(moth_anim PRIVATE -Wall -Werror)
endif()

target_precompile_headers(moth_anim PRIVATE src/common.h)

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_anim EXPORT moth_animTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_animTargets
        FILE moth_animTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_anim)

    configure_package_config_file(
        cmake/moth_animConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_animConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_anim)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_animConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_animConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_animConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_anim)
endif()
