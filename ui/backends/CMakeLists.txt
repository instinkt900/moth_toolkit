cmake_minimum_required(VERSION 3.10)

project(moth_ui_backends VERSION 1.0 DESCRIPTION "Rendering backends for Moth UI")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if (MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS _ITERATOR_DEBUG_LEVEL=0)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
endif()

add_subdirectory(.. moth_ui)

include_directories(../external/imgui)
include_directories(../external/murmurhash.c)
include_directories(../external/harfbuzz-icu-freetype/harfbuzz/src)

set(PUBLIC_HEADERS
	include/application.h
	include/events/event.h
	include/igraphics_context.h
	include/sdl/sdl_app.h
	include/sdl/sdl_events.h
	include/sdl/sdl_font.h
	include/sdl/SDL_FontCache.h
	include/sdl/sdl_font_factory.h
	include/sdl/sdl_graphics.h
	include/sdl/sdl_image.h
	include/sdl/sdl_image_factory.h
	include/sdl/sdl_ui_renderer.h
	include/sdl/sdl_utils.h
	include/sdl/smart_sdl.h
	include/vulkan/vulkan_app.h
	include/vulkan/vulkan_buffer.h
	include/vulkan/vulkan_command_buffer.h
	include/vulkan/vulkan_context.h
	include/vulkan/vulkan_events.h
	include/vulkan/vulkan_fence.h
	include/vulkan/vulkan_font.h
	include/vulkan/vulkan_font_cache.h
	include/vulkan/vulkan_font_factory.h
	include/vulkan/vulkan_framebuffer.h
	include/vulkan/vulkan_graphics.h
	include/vulkan/vulkan_image.h
	include/vulkan/vulkan_image_factory.h
	include/vulkan/vulkan_pipeline.h
	include/vulkan/vulkan_renderpass.h
	include/vulkan/vulkan_shader.h
	include/vulkan/vulkan_subimage.h
	include/vulkan/vulkan_swapchain.h
	include/vulkan/vulkan_ui_renderer.h
	include/vulkan/vulkan_utils.h
)

set(SOURCES ${SOURCES}
	../external/murmurhash.c/murmurhash.c
	src/application.cpp
	src/sdl/sdl_app.cpp
	src/sdl/sdl_events.cpp
	src/sdl/sdl_font.cpp
	src/sdl/SDL_FontCache.c
	src/sdl/sdl_font_factory.cpp
	src/sdl/sdl_graphics.cpp
	src/sdl/sdl_image.cpp
	src/sdl/sdl_image_factory.cpp
	src/sdl/sdl_ui_renderer.cpp
	src/stb_rect_pack.cpp
	src/vulkan/vulkan_app.cpp
	src/vulkan/vulkan_buffer.cpp
	src/vulkan/vulkan_command_buffer.cpp
	src/vulkan/vulkan_context.cpp
	src/vulkan/vulkan_events.cpp
	src/vulkan/vulkan_fence.cpp
	src/vulkan/vulkan_font.cpp
	src/vulkan/vulkan_font_cache.cpp
	src/vulkan/vulkan_font_factory.cpp
	src/vulkan/vulkan_framebuffer.cpp
	src/vulkan/vulkan_graphics.cpp
	src/vulkan/vulkan_image.cpp
	src/vulkan/vulkan_image_factory.cpp
	src/vulkan/vulkan_pipeline.cpp
	src/vulkan/vulkan_renderpass.cpp
	src/vulkan/vulkan_shader.cpp
	src/vulkan/vulkan_subimage.cpp
	src/vulkan/vulkan_swapchain.cpp
	src/vulkan/vulkan_ui_renderer.cpp
	src/vulkan/vulkan_vma.cpp
)

find_package(fmt REQUIRED)
find_package(freetype REQUIRED)
find_package(glfw3 REQUIRED)
find_package(magic_enum REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(range-v3 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(spdlog REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vulkan-memory-allocator REQUIRED)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} moth_ui)
target_link_libraries(${PROJECT_NAME} fmt::fmt)
target_link_libraries(${PROJECT_NAME} freetype)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} magic_enum::magic_enum)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} range-v3::range-v3)
target_link_libraries(${PROJECT_NAME} SDL2::SDL2main)
target_link_libraries(${PROJECT_NAME} SDL2_image::SDL2_image)
target_link_libraries(${PROJECT_NAME} SDL2_ttf::SDL2_ttf-static)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} vulkan-memory-allocator::vulkan-memory-allocator)

if (MSVC)
	set_source_files_properties(src/sdl/SDL_FontCache.c PROPERTIES COMPILE_FLAGS "/W0")
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Werror -Wall")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY "include/"
        DESTINATION "include"
        FILES_MATCHING
        PATTERN "*.h"
)
