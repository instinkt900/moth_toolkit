cmake_minimum_required(VERSION 3.27)

project(moth_profile
    VERSION 0.1.0
    DESCRIPTION "Frame profiling (scope timing and an ImGui profiler panel) for the Moth toolkit."
    LANGUAGES CXX)

# The panel is the only part that needs ImGui, which moth_graphics compiles in.
# Turn it off for a recorder-only build without the graphics stack.
option(MOTH_PROFILE_ENABLE_IMGUI "Build the moth::profile_imgui profiler panel (requires moth_graphics)" ON)

# ---- Recorder ----------------------------------------------------------------

add_library(moth_profile
    src/profiler.cpp)
target_compile_features(moth_profile PUBLIC cxx_std_17)
set_target_properties(moth_profile PROPERTIES EXPORT_NAME profile)

target_include_directories(moth_profile PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_profile PRIVATE src)

set(MOTH_PROFILE_TARGETS moth_profile)

# ---- ImGui panel -------------------------------------------------------------

if(MOTH_PROFILE_ENABLE_IMGUI)
    # Sibling module: in the superbuild the target already exists, standalone it
    # is located via find_package.
    if(NOT TARGET moth_graphics)
        find_package(moth_graphics CONFIG REQUIRED)
    endif()

    # Resolve the gfx link target: the namespaced package name (standalone / Conan),
    # the superbuild alias, or the bare in-tree target (source-built tests).
    if(TARGET moth_graphics::moth_graphics)
        set(MOTH_PROFILE_GFX moth_graphics::moth_graphics)
    elseif(TARGET moth::gfx)
        set(MOTH_PROFILE_GFX moth::gfx)
    else()
        set(MOTH_PROFILE_GFX moth_graphics)
    endif()

    add_library(moth_profile_imgui
        src/imgui/profiler_panel.cpp)
    target_compile_features(moth_profile_imgui PUBLIC cxx_std_17)
    set_target_properties(moth_profile_imgui PROPERTIES EXPORT_NAME profile_imgui)
    target_include_directories(moth_profile_imgui PRIVATE src)

    target_link_libraries(moth_profile_imgui PUBLIC moth_profile)
    target_link_libraries(moth_profile_imgui PUBLIC ${MOTH_PROFILE_GFX})

    list(APPEND MOTH_PROFILE_TARGETS moth_profile_imgui)
endif()

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS ${MOTH_PROFILE_TARGETS} EXPORT moth_profileTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_profileTargets
        FILE moth_profileTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_profile)

    configure_package_config_file(
        cmake/moth_profileConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_profileConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_profile)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_profileConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_profileConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_profileConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_profile)
endif()
