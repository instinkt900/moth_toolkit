# moth::bridge — adapter wiring moth::ui's renderer interfaces to moth::gfx.
#
# PLACEHOLDER: extracted from gfx/ during roadmap Phase 2. Lives outside both
# ui/ and gfx/ so neither module depends on the other.

add_library(moth_bridge INTERFACE)
add_library(moth::bridge ALIAS moth_bridge)

target_include_directories(moth_bridge INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# TODO(Phase 2): move gfx/graphics/moth_ui/ (moth_renderer.h, moth_image.h, ...)
#                here once moth::ui depends on moth::core rather than vice versa.
