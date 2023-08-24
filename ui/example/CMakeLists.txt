cmake_minimum_required(VERSION 3.10)

project(moth_example VERSION 1.0 DESCRIPTION "An example application using Moth UI")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if (MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
endif()

add_compile_definitions(_CRT_SECURE_NO_WARNINGS _ITERATOR_DEBUG_LEVEL=0 IMGUI_DEFINE_MATH_OPERATORS)

add_subdirectory(../backends moth_ui_backends)

include_directories(../backends/include)
include_directories(../external/imgui)
include_directories(../external/murmurhash.c)

set(SOURCES ${SOURCES}
	../external/imgui/imgui.cpp
	../external/imgui/imgui_draw.cpp
	../external/imgui/imgui_tables.cpp
	../external/imgui/imgui_widgets.cpp
	../external/imgui/backends/imgui_impl_sdlrenderer.cpp
	../external/imgui/backends/imgui_impl_sdl.cpp
	../external/imgui/backends/imgui_impl_vulkan.cpp
	../external/imgui/backends/imgui_impl_glfw.cpp
	src/example_application.cpp
	src/example_layer.cpp
	src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} moth_ui_backends)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
if (MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -Werror")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
