#pragma once

#include "moth_graphics/utils/vector.h"

namespace moth::gfx {
    namespace platform {
        class Window;
    }
    namespace graphics {
        class IGraphics;
        class ITexture;
    }
}

namespace moth::gfx::platform {

    class ImGuiContext {
    public:
        virtual ~ImGuiContext() = default;

        virtual void NewFrame() = 0;
        virtual void Render(graphics::IGraphics& graphics) = 0;
        virtual void Shutdown() = 0;

        /// @brief Draw a texture as an ImGui image widget.
        ///
        /// Tooling helper kept on the ImGui seam (rather than on @c ITexture) so
        /// the core texture resource stays free of ImGui concerns.
        /// @param texture Texture to draw.
        /// @param size    Display size in ImGui pixels.
        /// @param uv0     Top-left UV coordinate (texture-space, 0..1).
        /// @param uv1     Bottom-right UV coordinate (texture-space, 0..1).
        virtual void Image(graphics::ITexture const& texture, IntVec2 const& size,
                           FloatVec2 const& uv0, FloatVec2 const& uv1) = 0;
    };

}
