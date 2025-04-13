#pragma once

#include "application.h"

#include "moth_ui/context.h"
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

        std::shared_ptr<moth_ui::Context> m_uiContext;

    private:
        Graphics* m_vulkanGraphics = nullptr;
        GLFWwindow* m_glfwWindow = nullptr;
        std::unique_ptr<Context> m_context;
        VkSurfaceKHR m_customVkSurface = VK_NULL_HANDLE;
        moth_ui::FloatVec2 m_lastMousePos;
        bool m_haveMousePos = false;

        void ImGuiInit();
        void OnResize();
    };
}
