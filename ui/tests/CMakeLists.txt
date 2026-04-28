cmake_minimum_required(VERSION 3.15)

project(moth_ui_tests VERSION 1.0 DESCRIPTION "Unit tests for moth_ui")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 3 REQUIRED)

add_subdirectory(.. moth_ui)

set(SOURCES
    src/test_color.cpp
    src/test_vector.cpp
    src/test_rect.cpp
    src/test_vector_utils.cpp
    src/test_interp.cpp
    src/test_animation_clip.cpp
    src/test_animation_track.cpp
    src/test_animation_marker.cpp
    src/test_layout_rect.cpp
    src/test_event_key.cpp
    src/test_event_dispatch.cpp
    src/test_node.cpp
    src/test_node_entity.cpp
    src/test_group.cpp
    src/test_layout_serialization.cpp
    src/test_animation_clip_controller.cpp
    src/test_transform.cpp
    src/test_node_flipbook.cpp
    src/test_discrete_animation_track.cpp
    src/test_discrete_animation_track_controller.cpp
    src/test_logger.cpp
    src/test_node_factory.cpp
    # API surface tests — compile-time only, no runtime assertions
    src/api_surface_umbrella.cpp
    src/api_surface_fwd.cpp
    src/api_surface_graphics.cpp
    src/api_surface_factories.cpp
    src/api_surface_nodes.cpp
    src/api_surface_animation.cpp
    src/api_surface_layout.cpp
    src/api_surface_events.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE moth_ui Catch2::Catch2WithMain)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
    target_link_options(${PROJECT_NAME} PRIVATE /ignore:4099)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
