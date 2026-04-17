cmake_minimum_required(VERSION 3.10)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" VERSION_STR)
string(STRIP "${VERSION_STR}" PROJECT_VERSION_FULL)
# CMake's VERSION field requires numeric x.y.z — strip any pre-release suffix (e.g. -rc.1)
string(REGEX REPLACE "[-+].*$" "" PROJECT_VERSION "${PROJECT_VERSION_FULL}")

project(moth_ui VERSION ${PROJECT_VERSION} DESCRIPTION "A C++ game UI library")

configure_file(
    include/moth_ui/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/moth_ui/version.h
)

message("Building with CMake version: ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PUBLIC_HEADERS
	include/moth_ui/animation/animation_clip_controller.h
	include/moth_ui/animation/animation_clip.h
	include/moth_ui/animation/animation_controller.h
	include/moth_ui/animation/animation_event.h
	include/moth_ui/animation/animation_track_controller.h
	include/moth_ui/animation/animation_track.h
	include/moth_ui/animation/clip_controller.h
	include/moth_ui/animation/discrete_animation_track.h
	include/moth_ui/animation/discrete_animation_track_controller.h
	include/moth_ui/animation/keyframe.h
	include/moth_ui/context.h
	include/moth_ui/events/event_animation.h
	include/moth_ui/events/event_dispatch.h
	include/moth_ui/events/event_flipbook.h
	include/moth_ui/events/event.h
	include/moth_ui/events/event_key.h
	include/moth_ui/events/event_listener.h
	include/moth_ui/events/event_mouse.h
	include/moth_ui/font_factory.h
	include/moth_ui/graphics/blend_mode.h
	include/moth_ui/graphics/iflipbook.h
	include/moth_ui/graphics/ifont.h
	include/moth_ui/graphics/iimage.h
	include/moth_ui/graphics/image_scale_type.h
	include/moth_ui/graphics/irenderer.h
	include/moth_ui/graphics/itarget.h
	include/moth_ui/graphics/text_alignment.h
	include/moth_ui/iflipbook_factory.h
	include/moth_ui/ifont_factory.h
	include/moth_ui/iimage_factory.h
	include/moth_ui/ilogger.h
	include/moth_ui/layers/layer.h
	include/moth_ui/layers/layer_stack.h
	include/moth_ui/layout/layout_cache.h
	include/moth_ui/layout/layout_entity_clip.h
	include/moth_ui/layout/layout_entity_flipbook.h
	include/moth_ui/layout/layout_entity_group.h
	include/moth_ui/layout/layout_entity.h
	include/moth_ui/layout/layout_entity_image.h
	include/moth_ui/layout/layout_entity_rect.h
	include/moth_ui/layout/layout_entity_ref.h
	include/moth_ui/layout/layout_entity_text.h
	include/moth_ui/layout/layout_entity_type.h
	include/moth_ui/layout/layout.h
	include/moth_ui/layout/layout_rect.h
	include/moth_ui/node_factory.h
	include/moth_ui/nodes/group.h
	include/moth_ui/nodes/node_clip.h
	include/moth_ui/nodes/node_flipbook.h
	include/moth_ui/nodes/node.h
	include/moth_ui/nodes/node_image.h
	include/moth_ui/nodes/node_rect.h
	include/moth_ui/nodes/node_text.h
	include/moth_ui/nodes/widget.h
	include/moth_ui/utils/color.h
	include/moth_ui/utils/interp.h
	include/moth_ui/utils/rect.h
	include/moth_ui/utils/rect_serialization.h
	include/moth_ui/utils/serialize_utils.h
	include/moth_ui/utils/vector.h
	include/moth_ui/utils/vector_serialization.h
)
set(SOURCES ${SOURCES}
	src/animation/animation_clip_controller.cpp
	src/animation/animation_controller.cpp
	src/animation/animation_event.cpp
	src/animation/animation_track_controller.cpp
	src/animation/animation_track.cpp
	src/animation/discrete_animation_track.cpp
	src/animation/discrete_animation_track_controller.cpp
	src/animation/keyframe.cpp
	src/context.cpp
	src/logger.cpp
	src/font_factory.cpp
	src/layers/layer.cpp
	src/layers/layer_stack.cpp
	src/layout/layout_cache.cpp
	src/layout/layout.cpp
	src/layout/layout_entity_clip.cpp
	src/layout/layout_entity.cpp
	src/layout/layout_entity_flipbook.cpp
	src/layout/layout_entity_group.cpp
	src/layout/layout_entity_image.cpp
	src/layout/layout_entity_rect.cpp
	src/layout/layout_entity_ref.cpp
	src/layout/layout_entity_text.cpp
	src/node_factory.cpp
	src/nodes/group.cpp
	src/nodes/node_clip.cpp
	src/nodes/node.cpp
	src/nodes/node_flipbook.cpp
	src/nodes/node_image.cpp
	src/nodes/node_rect.cpp
	src/nodes/node_text.cpp
)

find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(magic_enum REQUIRED)
find_package(range-v3 REQUIRED)

add_library(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum::magic_enum)
target_link_libraries(${PROJECT_NAME} PUBLIC range-v3::range-v3)

if (MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")
	target_compile_definitions(${PROJECT_NAME} PRIVATE _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Werror -Wall")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR DEFINED ENV{CI})
	find_program(CLANG_TIDY_EXE NAMES clang-tidy)
	if(CLANG_TIDY_EXE)
		set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
	elseif(DEFINED ENV{CI})
		message(FATAL_ERROR "clang-tidy not found but required in CI")
	else()
		message(STATUS "clang-tidy not found, skipping")
	endif()
endif()

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
)
install(DIRECTORY "include/"
	DESTINATION "include"
	FILES_MATCHING PATTERN "*.h"
)

find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(DOXYGEN_FOUND)
	include(FetchContent)
	FetchContent_Declare(
		doxygen-awesome-css
		GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
		GIT_TAG v2.4.2
	)
	FetchContent_MakeAvailable(doxygen-awesome-css)

	set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
	set(DOXYGEN_RECURSIVE YES)
	set(DOXYGEN_EXTRACT_ALL YES)
	set(DOXYGEN_GENERATE_TREEVIEW YES)
	set(DOXYGEN_HTML_COLORSTYLE AUTO)
	set(DOXYGEN_HTML_EXTRA_STYLESHEET ${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css)
	doxygen_add_docs(docs
		${CMAKE_CURRENT_SOURCE_DIR}/include
		COMMENT "Generating API documentation"
	)
else()
	add_custom_target(docs
		COMMAND ${CMAKE_COMMAND} -E echo "docs target unavailable: doxygen not found. Install doxygen and reconfigure."
	)
endif()
