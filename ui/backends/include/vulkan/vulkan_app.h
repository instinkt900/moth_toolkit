#pragma once

#include "application.h"

#include "vulkan/vulkan_context.h"
#include "vulkan/vulkan_graphics.h"
#include "vulkan/vulkan_swapchain.h"

#include <GLFW/glfw3.h>

namespace backend::vulkan {
    class Application : public backend::Application {
    public:
        Application(std::string const& applicationTitle);
        virtual ~Application();

        void SetWindowTitle(std::string const& title) override;

    protected:
        bool CreateWindow() override;
        void DestroyWindow() override;
        void UpdateWindow() override;
        void Draw() override;

    private:
        Graphics* m_vulkanGraphics = nullptr;
        GLFWwindow* m_glfwWindow = nullptr;
        std::unique_ptr<Context> m_context;
        bool m_vkSwapChainrebuild = false;
        VkSurfaceKHR m_customVkSurface = VK_NULL_HANDLE;

        void ImGuiInit();
        void OnResize();
    };
}
