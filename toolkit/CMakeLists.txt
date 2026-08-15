# moth::toolkit — aggregate target linking every enabled module.
#
# Consumers link moth::toolkit to get "everything" with the feature flags baked
# in. The same modules can also be linked individually (moth::core, moth::gfx,
# ...) for leaner consumers.

add_library(moth_toolkit INTERFACE)
add_library(moth::toolkit ALIAS moth_toolkit)

# Expose the generated feature header and the toolkit umbrella header to any
# consumer of the toolkit.
target_include_directories(moth_toolkit INTERFACE
    ${CMAKE_BINARY_DIR}/generated/include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

if(MOTH_ENABLE_CORE)
    target_link_libraries(moth_toolkit INTERFACE moth::core)
endif()

if(MOTH_ENABLE_GFX)
    target_link_libraries(moth_toolkit INTERFACE moth::gfx)
endif()

if(MOTH_ENABLE_UI)
    target_link_libraries(moth_toolkit INTERFACE moth::ui)
endif()

if(MOTH_ENABLE_BRIDGE)
    target_link_libraries(moth_toolkit INTERFACE moth::bridge)
endif()

install(DIRECTORY include/ DESTINATION include)
