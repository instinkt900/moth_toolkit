#pragma once

#include "moth/graphics/graphics/igraphics.h"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <cstdint>

namespace moth::gfx::vulkan {
    class SurfaceContext;

    /// @brief Creation-time settings for the Vulkan graphics backend.
    struct GraphicsSettings {
        /// @brief Number of vertices each draw context can queue in a frame.
        ///
        /// When the buffer fills, the frame is submitted and the CPU waits for
        /// the GPU before continuing, which is expensive. Raise this for scenes
        /// that draw many sprites or tiles per frame (a quad is 6 vertices).
        /// Each draw context allocates capacity * 32 bytes of host-visible
        /// memory, about 2 MB at the default. Values below one quad are clamped up.
        uint32_t vertexBufferCapacity = 65536;
    };

    /// @brief Create a Vulkan graphics instance for standalone use.
    ///
    /// Constructs the Vulkan backend renderer without requiring Window or
    /// Application. Begin() self-heals when the swapchain is unavailable
    /// (e.g. window minimised) — callers do not need to handle resize.
    ///
    /// Power users can interleave their own Vulkan commands via
    /// vulkan::Graphics::Flush() and GetCurrentCommandBuffer() (accessible
    /// by downcasting the returned IGraphics).
    ///
    /// @param surfaceContext  Initialized Vulkan surface context.
    /// @param surface         Vulkan surface to render into.
    /// @param surfaceWidth    Initial surface width in pixels.
    /// @param surfaceHeight   Initial surface height in pixels.
    /// @param settings        Backend settings such as the vertex buffer capacity.
    /// @returns A fully initialized graphics instance.
    std::unique_ptr<IGraphics> CreateGraphics(
        SurfaceContext& surfaceContext,
        VkSurfaceKHR surface,
        uint32_t surfaceWidth,
        uint32_t surfaceHeight,
        GraphicsSettings const& settings = {});
}
