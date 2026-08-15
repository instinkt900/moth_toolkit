#pragma once

#include "moth_graphics/graphics/surface_context.h"
#include "moth_graphics/platform/window.h"

#include <moth/core/glfw/window.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <string_view>
#include <cstdint>

namespace moth_graphics::graphics::vulkan {
    struct Context;
    class SurfaceContext;
}

namespace moth_graphics::platform::glfw {
    /// @brief A Vulkan/GLFW window with rendering (no UI).
    ///
    /// Composes a @c moth::core::glfw::Window for the native window/input and
    /// adds the Vulkan surface and graphics context. Input events are forwarded
    /// to the UI delegate installed via @c Window::SetUiDelegate.
    class Window : public moth_graphics::platform::Window, public moth::core::glfw::Window::Listener {
    public:
        Window(graphics::vulkan::Context& context, std::string_view title, int width, int height);
        ~Window() override;

        graphics::SurfaceContext& GetSurfaceContext() const override;
        void SetWindowTitle(std::string_view title) override;
        GLFWwindow* GetGLFWWindow() const { return m_nativeWindow->GetGLFWWindow(); }
        VkSurfaceKHR GetVkSurface() const { return m_customVkSurface; }

        void Update(uint32_t ticks) override;
        void BeginFrame() override;
        void EndFrame() override;

        // Native-window state delegated to the composed core window.
        bool IsMaximized() const override;
        moth::core::IntVec2 const& GetPosition() const override;
        int GetWidth() const override;
        int GetHeight() const override;

        // core::glfw::Window::Listener (also overrides core::Window / IEventListener).
        moth::core::IntVec2 GetRenderSize() const override;
        bool OnEvent(moth::core::Event const& event) override;
        void OnResize(int width, int height) override;

    private:
        bool CreateSurface();

        graphics::vulkan::Context& m_context;
        std::unique_ptr<moth::core::glfw::Window> m_nativeWindow;
        std::unique_ptr<graphics::vulkan::SurfaceContext> m_surfaceContext;
        VkSurfaceKHR m_customVkSurface = VK_NULL_HANDLE;
    };
}
