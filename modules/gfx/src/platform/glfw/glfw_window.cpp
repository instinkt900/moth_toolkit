#include "graphics/vulkan/vulkan_utils.h"
#include "common.h"
#include "moth_graphics/platform/glfw/glfw_window.h"
#include "graphics/vulkan/vulkan_graphics.h"
#include "moth_graphics/graphics/vulkan/vulkan_surface_context.h"

#include <cassert>

namespace moth::gfx::platform::glfw {
    Window::Window(graphics::vulkan::Context& context, std::string_view title, int width, int height)
        : moth::gfx::platform::Window(title, width, height)
        , m_context(context) {
        m_nativeWindow = std::make_unique<moth::core::glfw::Window>(title, width, height);
        m_nativeWindow->SetListener(this);
        CreateSurface();
    }

    Window::~Window() {
        if (m_surfaceContext) {
            vkDeviceWaitIdle(m_surfaceContext->GetVkDevice());
        }
        SetGraphics(nullptr);      // vulkan::Graphics (uses surface, pool)
        m_surfaceContext.reset();
        if (m_customVkSurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_context.instance, m_customVkSurface, nullptr);
            m_customVkSurface = VK_NULL_HANDLE;
        }
        m_nativeWindow.reset();    // destroys the GLFW window
    }

    graphics::SurfaceContext& Window::GetSurfaceContext() const {
        assert(m_surfaceContext && "GetSurfaceContext called on a window without a valid surface context (CreateWindow failed or already destroyed)");
        return *m_surfaceContext;
    }

    void Window::Update(uint32_t ticks) {
        m_nativeWindow->Update(ticks);
    }

    void Window::BeginFrame() {
        GetGraphics().Begin();
    }

    void Window::EndFrame() {
        GetGraphics().End();
    }

    void Window::SetWindowTitle(std::string_view title) {
        m_nativeWindow->SetWindowTitle(title);
    }

    bool Window::IsMaximized() const {
        return m_nativeWindow->IsMaximized();
    }

    moth::core::IntVec2 const& Window::GetPosition() const {
        return m_nativeWindow->GetPosition();
    }

    int Window::GetWidth() const {
        return m_nativeWindow->GetWidth();
    }

    int Window::GetHeight() const {
        return m_nativeWindow->GetHeight();
    }

    moth::core::IntVec2 Window::GetRenderSize() const {
        auto* delegate = GetUiDelegate();
        return delegate ? delegate->GetRenderSize() : moth::core::Window::GetRenderSize();
    }

    bool Window::OnEvent(moth::core::Event const& event) {
        auto* delegate = GetUiDelegate();
        return delegate ? delegate->OnEvent(event) : EmitEvent(event);
    }

    void Window::OnResize(int width, int height) {
        auto* graphics = dynamic_cast<graphics::vulkan::Graphics*>(GetGraphicsPtr());
        if (graphics == nullptr) {
            return;
        }
        graphics->OnResize(m_customVkSurface, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }

    bool Window::CreateSurface() {
        moth::core::log::info("GLFW: creating window '{}' ({}x{})", m_title, GetWidth(), GetHeight());
        CHECK_VK_RESULT(glfwCreateWindowSurface(m_context.instance, m_nativeWindow->GetGLFWWindow(), nullptr, &m_customVkSurface));
        m_surfaceContext = std::make_unique<graphics::vulkan::SurfaceContext>(m_context);

        SetGraphics(std::make_unique<graphics::vulkan::Graphics>(*m_surfaceContext, m_customVkSurface, GetWidth(), GetHeight()));
        moth::core::log::info("GLFW: window '{}' ready", m_title);
        return true;
    }
}
