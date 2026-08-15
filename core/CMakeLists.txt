# moth::core — game-loop / platform / math / events foundation.
#
# PLACEHOLDER: this module is extracted from gfx/ during roadmap Phase 1
# (platform/, Ticker, events, math/vector/rect, logging). It currently declares
# an empty INTERFACE target so the superbuild and the toolkit aggregate have a
# stable shape to wire against.

add_library(moth_core INTERFACE)
add_library(moth::core ALIAS moth_core)

target_include_directories(moth_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# TODO(Phase 1): add the real sources, PUBLIC compile definitions, and
# install(TARGETS ... EXPORT ...) so moth::core is independently consumable.
