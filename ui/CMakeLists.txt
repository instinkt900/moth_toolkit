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

set(SOURCES ${SOURCES}
	src/layout/layout.cpp
	src/layout/layout_cache.cpp
	src/layout/layout_entity.cpp
	src/layout/layout_entity_clip.cpp
	src/layout/layout_entity_group.cpp
	src/layout/layout_entity_image.cpp
	src/layout/layout_entity_rect.cpp
	src/layout/layout_entity_ref.cpp
	src/layout/layout_entity_text.cpp
	src/animation_controller.cpp
	src/animation_track.cpp
	src/context.cpp
	src/group.cpp
	src/node.cpp
	src/node_clip.cpp
	src/node_factory.cpp
	src/node_image.cpp
	src/node_rect.cpp
	src/node_text.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")
