cmake_minimum_required(VERSION 3.10)

project(moth_ui VERSION 1.0 DESCRIPTION "A C++ game UI library")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_compile_definitions(_CRT_SECURE_NO_WARNINGS _ITERATOR_DEBUG_LEVEL=0)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(external/imgui)

set(SOURCES ${SOURCES}
	external/imgui/imgui.cpp
	external/imgui/imgui_draw.cpp
	external/imgui/imgui_tables.cpp
	external/imgui/imgui_widgets.cpp
	src/animation_controller.cpp
	src/animation_track.cpp
	src/context.cpp
	src/group.cpp
	src/imgui_ext/imgui_ext.cpp
	src/imgui_ext/imgui_ext_focus.cpp
	src/layouts/layout.cpp
	src/layouts/layout_entity.cpp
	src/layouts/layout_entity_group.cpp
	src/layouts/layout_entity_image.cpp
	src/node.cpp
	src/node_image.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")
