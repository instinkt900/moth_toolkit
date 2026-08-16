cmake_minimum_required(VERSION 3.10)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" VERSION_STR)
string(STRIP "${VERSION_STR}" PROJECT_VERSION_FULL)
string(REGEX REPLACE "[-+].*$" "" PROJECT_VERSION "${PROJECT_VERSION_FULL}")

project(moth_graphics VERSION ${PROJECT_VERSION} DESCRIPTION "A basic graphical framework.")

configure_file(
    include/moth_graphics/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/moth_graphics/version.h
)

message("Building with CMake version: ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MOTH_GRAPHICS_DISABLE_VULKAN "Disable the Vulkan/GLFW backend" OFF)
option(MOTH_GRAPHICS_ENABLE_GLSLANG "Enable runtime GLSL compilation for custom shaders (requires glslang)" OFF)

if(MOTH_GRAPHICS_DISABLE_VULKAN)
    message(FATAL_ERROR "Vulkan is the only backend; it cannot be disabled (a future backend can re-introduce the toggle).")
endif()

# ---- Always-compiled sources ------------------------------------------------
set(SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    src/graphics/image.cpp
    src/graphics/polygon_triangulation.cpp
    src/graphics/sprite.cpp
    src/graphics/sprite_batch.cpp
    src/graphics/spritesheet.cpp
    src/graphics/spritesheet_factory.cpp
    src/graphics/font_factory.cpp
    src/graphics/texture_factory.cpp
    src/graphics/shader_factory.cpp
    src/graphics/stb_image.cpp
    src/graphics/stb_image_write.cpp
    src/graphics/stb_rect_pack.cpp
    src/game/game.cpp
    src/platform/window.cpp
    external/murmurhash.c/murmurhash.c
)

# ---- Vulkan backend sources --------------------------------------------------
if(NOT MOTH_GRAPHICS_DISABLE_VULKAN)
    list(APPEND SOURCES
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_vulkan.cpp
        src/graphics/vulkan/shaders/vulkan_shaders.cpp
        src/graphics/vulkan/vulkan_buffer.cpp
        src/graphics/vulkan/vulkan_command_buffer.cpp
        src/graphics/vulkan/vulkan_managed_context.cpp
        src/graphics/vulkan/vulkan_fence.cpp
        src/graphics/vulkan/vulkan_font.cpp
        src/graphics/vulkan/vulkan_framebuffer.cpp
        src/graphics/vulkan/vulkan_graphics.cpp
        src/graphics/vulkan/vulkan_graphics_frame.cpp
        src/graphics/vulkan/vulkan_graphics_pipeline.cpp
        src/graphics/vulkan/vulkan_graphics_shader.cpp
        src/graphics/vulkan/vulkan_pipeline.cpp
        src/graphics/vulkan/vulkan_renderpass.cpp
        src/graphics/vulkan/vulkan_shader.cpp
        src/graphics/vulkan/vulkan_shader_object.cpp
        src/graphics/vulkan/glsl_compiler.cpp
        src/graphics/vulkan/vulkan_asset_context.cpp
        src/graphics/vulkan/vulkan_surface_context.cpp
        src/graphics/vulkan/vulkan_swapchain.cpp
        src/graphics/vulkan/vulkan_texture.cpp
        src/graphics/vulkan/vulkan_vma.cpp
        src/platform/glfw/glfw_platform.cpp
        src/platform/glfw/glfw_window.cpp
    )
endif()

# ---- Dependencies ------------------------------------------------------------

# GLFW/FreeType/HarfBuzz must come from the system on Linux to avoid runtime
# symbol conflicts with GTK3/GDK-Pixbuf (used by NFD's file dialog).
if(NOT MOTH_GRAPHICS_DISABLE_VULKAN)
    if(WIN32)
        find_package(glfw3 REQUIRED)
        find_package(Freetype REQUIRED)
        find_package(harfbuzz REQUIRED)
        set(MOTH_GRAPHICS_GLFW_LINK glfw)
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GLFW3 REQUIRED IMPORTED_TARGET glfw3)
        set(MOTH_GRAPHICS_GLFW_LINK PkgConfig::GLFW3)
        find_package(Freetype REQUIRED)
        find_package(harfbuzz REQUIRED)
    endif()
    find_package(Vulkan REQUIRED)
    find_package(VulkanMemoryAllocator REQUIRED)
endif()

if(MOTH_GRAPHICS_ENABLE_GLSLANG)
    find_package(glslang REQUIRED)
endif()

# Core is a sibling module; in the superbuild the target already exists, standalone it is located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()

# ---- Target ------------------------------------------------------------------

add_library(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/imgui>
    $<INSTALL_INTERFACE:external/imgui>
)
target_include_directories(${PROJECT_NAME} PRIVATE external/murmurhash.c)
target_include_directories(${PROJECT_NAME} PRIVATE external/stb)
target_include_directories(${PROJECT_NAME} PRIVATE src)

if(NOT MOTH_GRAPHICS_DISABLE_VULKAN)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MOTH_GRAPHICS_DISABLE_VULKAN=0)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${MOTH_GRAPHICS_GLFW_LINK})
    target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)
    target_link_libraries(${PROJECT_NAME} PUBLIC GPUOpen::VulkanMemoryAllocator)
    target_link_libraries(${PROJECT_NAME} PUBLIC Freetype::Freetype)
    target_link_libraries(${PROJECT_NAME} PUBLIC harfbuzz::harfbuzz)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC MOTH_GRAPHICS_DISABLE_VULKAN=1)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC moth::core)

if(MOTH_GRAPHICS_ENABLE_GLSLANG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MOTH_GRAPHICS_ENABLE_GLSLANG=1)
    target_link_libraries(${PROJECT_NAME} PUBLIC glslang::glslang glslang::SPIRV glslang::glslang-default-resource-limits)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE MOTH_GRAPHICS_ENABLE_GLSLANG=0)
endif()

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -Werror")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR DEFINED ENV{CI})
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        # --extra-arg=-Wno-ignored-gch: clang-tidy uses Clang, which cannot read
        # GCC-format PCH files. Suppressing the warning entirely (not just -Wno-error)
        # prevents clang-tidy's WarningsAsErrors from re-promoting it to an error.
        set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--extra-arg=-Wno-ignored-gch")
        # Third-party source files — skip all linting (requires CMake 3.27+)
        set_source_files_properties(
            external/imgui/imgui.cpp
            external/imgui/imgui_draw.cpp
            external/imgui/imgui_tables.cpp
            external/imgui/imgui_widgets.cpp
            external/imgui/backends/imgui_impl_glfw.cpp
            external/imgui/backends/imgui_impl_vulkan.cpp
            external/murmurhash.c/murmurhash.c
            src/graphics/vulkan/shaders/vulkan_shaders.cpp
            PROPERTIES SKIP_LINTING ON
        )
    elseif(DEFINED ENV{CI})
        message(FATAL_ERROR "clang-tidy not found but required in CI")
    else()
        message(STATUS "clang-tidy not found, skipping")
    endif()
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE src/common.h)
set_source_files_properties(
    external/murmurhash.c/murmurhash.c
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp
    src/graphics/vulkan/shaders/vulkan_shaders.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

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
install(DIRECTORY "external/imgui/"
    DESTINATION "external/imgui"
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)
