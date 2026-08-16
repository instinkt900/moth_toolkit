#include "common.h"
#include "moth_graphics/platform/window.h"
#include "moth_graphics/graphics/surface_context.h"

#include <cassert>

namespace moth::gfx::platform {
    Window::Window(std::string_view title, int width, int height)
        : moth::core::Window(title, width, height) {
    }

    Window::~Window() = default;

    moth::gfx::TextureFactory& Window::GetTextureFactory() const {
        return GetSurfaceContext().GetAssetContext().GetTextureFactory();
    }

    moth::gfx::IGraphicsDevice& Window::GetDevice() const {
        // The graphics implementation doubles as the device (resource-creation)
        // backend; the interfaces are split so drawing and resource creation can
        // diverge later (e.g. a shared device for a future 3D backend).
        auto* device = dynamic_cast<moth::gfx::IGraphicsDevice*>(m_graphics.get());
        assert(device != nullptr && "GetDevice called on a window whose graphics backend is not an IGraphicsDevice");
        return *device;
    }
}
