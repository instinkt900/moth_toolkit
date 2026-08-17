#pragma once

#include "moth/graphics/platform/iplatform.h"
#include "moth/graphics/platform/window.h"

#include <memory>

namespace moth::gfx::vulkan {
    struct Context;
    class ManagedContext;
}

namespace moth::gfx::platform::glfw {
    class Platform : public IPlatform {
    public:
        Platform();
        ~Platform() noexcept override;

        bool Startup() override;
        void Shutdown() override;

        std::unique_ptr<moth::gfx::platform::Window> CreateWindow(std::string_view title, int width, int height) override;

        std::unique_ptr<ImGuiContext> CreateImGuiContext(platform::Window& window, moth::gfx::IGraphics& graphics, bool enableViewports) override;

    private:
        void ShutdownImpl();

        // No in-class `= nullptr` initializer: default-constructing
        // unique_ptr already nulls it, and an in-class default member init
        // forces the destructor of unique_ptr<ManagedContext> to instantiate
        // in any TU that processes this class body (e.g. moth_editor's
        // make_unique<Platform> call site), which then fails the incomplete-
        // type sizeof check on ManagedContext. The destructor body for
        // Platform is defined in glfw_platform.cpp where the type is complete.
        std::unique_ptr<moth::gfx::vulkan::ManagedContext> m_context;
        bool m_initialized = false;
    };
}
