#pragma once

#include "moth_graphics/graphics/itarget.h"

#include <memory>

namespace moth::gfx::graphics {
    /// @brief Creates backend GPU resources, decoupled from drawing.
    ///
    /// @c IGraphics is immediate-mode 2D drawing only; this interface owns
    /// resource creation so a future command interface (e.g. a 3D backend) can
    /// share the same device instead of re-implementing the platform layer.
    ///
    /// Texture and font loading already live on @c AssetContext (obtained via
    /// @c SurfaceContext::GetAssetContext()); render-target creation lives here.
    class IGraphicsDevice {
    public:
        virtual ~IGraphicsDevice() = default;

        /// @brief Create an off-screen render target.
        ///
        /// Activate it with @c IGraphics::SetTarget() to redirect draw calls,
        /// then read back the result through @c ITarget::GetImage().
        /// @param width Width in pixels.
        /// @param height Height in pixels.
        /// @returns Ownership of a new render target.
        virtual std::unique_ptr<ITarget> CreateTarget(int width, int height) = 0;
    };
}
