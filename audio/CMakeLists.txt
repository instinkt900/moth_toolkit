cmake_minimum_required(VERSION 3.27)

project(moth_audio
    VERSION 0.1.0
    DESCRIPTION "A lean miniaudio wrapper for sound and music playback in the Moth toolkit."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(miniaudio CONFIG REQUIRED)

# Sibling module: in the superbuild the target already exists, standalone it is
# located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()

add_library(moth_audio src/audio.cpp)
set_target_properties(moth_audio PROPERTIES EXPORT_NAME audio)

target_include_directories(moth_audio PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_audio PRIVATE src)

target_link_libraries(moth_audio PUBLIC miniaudio::miniaudio)
target_link_libraries(moth_audio PUBLIC moth::core)

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_audio EXPORT moth_audioTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_audioTargets
        FILE moth_audioTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_audio)

    configure_package_config_file(
        cmake/moth_audioConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_audioConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_audio)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_audioConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_audioConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_audioConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_audio)
endif()
