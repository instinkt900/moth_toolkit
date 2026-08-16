cmake_minimum_required(VERSION 3.27)

project(moth_bridge
    VERSION 0.1.0
    DESCRIPTION "Adapter wiring moth_ui and the graphics window together."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sibling modules: in the superbuild the targets already exist, standalone they
# are located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()
if(NOT TARGET moth_graphics)
    find_package(moth_graphics CONFIG REQUIRED)
endif()
if(NOT TARGET moth_ui)
    find_package(moth_ui CONFIG REQUIRED)
endif()
find_package(spdlog REQUIRED)

set(SOURCES
    src/moth_flipbook.cpp
    src/moth_flipbook_factory.cpp
    src/moth_font_factory.cpp
    src/moth_image.cpp
    src/moth_image_factory.cpp
    src/moth_renderer.cpp
    src/ui_window.cpp
    src/application.cpp
)

add_library(moth_bridge ${SOURCES})
add_library(moth::bridge ALIAS moth_bridge)
set_target_properties(moth_bridge PROPERTIES EXPORT_NAME bridge)

target_include_directories(moth_bridge PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(moth_bridge PUBLIC moth::core)
target_link_libraries(moth_bridge PUBLIC moth_graphics::moth_graphics)
target_link_libraries(moth_bridge PUBLIC moth_ui::moth_ui)
target_link_libraries(moth_bridge PUBLIC spdlog::spdlog)

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(GNUInstallDirs)
    install(TARGETS moth_bridge EXPORT moth_bridgeTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_bridgeTargets
        FILE moth_bridgeTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_bridge)
endif()
