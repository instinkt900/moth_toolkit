cmake_minimum_required(VERSION 3.10)

project(moth_ui VERSION 1.0 DESCRIPTION "A C++ game UI library")

message("Building with CMake version: ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PUBLIC_HEADERS
	include/moth_ui/animation_clip.h
	include/moth_ui/animation_controller.h
	include/moth_ui/animation_track.h
	include/moth_ui/blend_mode.h
	include/moth_ui/clip_controller.h
	include/moth_ui/context.h
	include/moth_ui/events/event.h
	include/moth_ui/events/event_animation.h
	include/moth_ui/events/event_key.h
	include/moth_ui/events/event_mouse.h
	include/moth_ui/event_dispatch.h
	include/moth_ui/event_listener.h
	include/moth_ui/font_factory.h
	include/moth_ui/group.h
	include/moth_ui/ifont.h
	include/moth_ui/iimage.h
	include/moth_ui/iimage_factory.h
	include/moth_ui/image_scale_type.h
	include/moth_ui/irenderer.h
	include/moth_ui/itarget.h
	include/moth_ui/layers/layer.h
	include/moth_ui/layers/layer_stack.h
	include/moth_ui/layout/layout.h
	include/moth_ui/layout/layout_cache.h
	include/moth_ui/layout/layout_entity.h
	include/moth_ui/layout/layout_entity_clip.h
	include/moth_ui/layout/layout_entity_group.h
	include/moth_ui/layout/layout_entity_image.h
	include/moth_ui/layout/layout_entity_rect.h
	include/moth_ui/layout/layout_entity_ref.h
	include/moth_ui/layout/layout_entity_text.h
	include/moth_ui/layout/layout_entity_type.h
	include/moth_ui/layout/layout_rect.h
	include/moth_ui/node.h
	include/moth_ui/node_clip.h
	include/moth_ui/node_factory.h
	include/moth_ui/node_image.h
	include/moth_ui/node_rect.h
	include/moth_ui/node_text.h
	include/moth_ui/text_alignment.h
	include/moth_ui/ui_fwd.h
	include/moth_ui/utils/color.h
	include/moth_ui/utils/interp.h
	include/moth_ui/utils/math_utils.h
	include/moth_ui/utils/rect.h
	include/moth_ui/utils/rect_serialization.h
	include/moth_ui/utils/serialize_utils.h
	include/moth_ui/utils/vector.h
	include/moth_ui/utils/vector_serialization.h
	include/moth_ui/widget.h
)

set(SOURCES ${SOURCES}
	src/animation_controller.cpp
	src/animation_track.cpp
	src/context.cpp
	src/font_factory.cpp
	src/group.cpp
	src/layers/layer.cpp
	src/layers/layer_stack.cpp
	src/layout/layout.cpp
	src/layout/layout_cache.cpp
	src/layout/layout_entity.cpp
	src/layout/layout_entity_clip.cpp
	src/layout/layout_entity_group.cpp
	src/layout/layout_entity_image.cpp
	src/layout/layout_entity_rect.cpp
	src/layout/layout_entity_ref.cpp
	src/layout/layout_entity_text.cpp
	src/node.cpp
	src/node_clip.cpp
	src/node_factory.cpp
	src/node_image.cpp
	src/node_rect.cpp
	src/node_text.cpp
)

find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(magic_enum REQUIRED)
find_package(range-v3 REQUIRED)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} fmt::fmt)
target_link_libraries(${PROJECT_NAME} magic_enum::magic_enum)
target_link_libraries(${PROJECT_NAME} range-v3::range-v3)

if (MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Werror -Wall")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY "include/" # source directory
        DESTINATION "include" # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
)
