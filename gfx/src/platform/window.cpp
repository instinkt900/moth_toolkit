#include "common.h"
#include "moth_graphics/platform/window.h"
#include "moth_graphics/graphics/surface_context.h"

namespace moth::gfx::platform {
    Window::Window(std::string_view title, int width, int height)
        : moth::core::Window(title, width, height) {
    }

    Window::~Window() = default;

    graphics::TextureFactory& Window::GetTextureFactory() const {
        return GetSurfaceContext().GetAssetContext().GetTextureFactory();
    }
}
