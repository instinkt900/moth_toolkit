#include "common.h"
#include "vulkan/vulkan_app.h"

#include "vulkan/vulkan_ui_renderer.h"
#include "vulkan/vulkan_image_factory.h"
#include "vulkan/vulkan_font_factory.h"
#include "vulkan/vulkan_graphics.h"
#include "vulkan/vulkan_events.h"

#include "moth_ui/context.h"

namespace {
    void checkVkResult(VkResult err) {
        CHECK_VK_RESULT(err);
    }
}

namespace backend::vulkan {
    Application::Application(std::string const& applicationTitle)
        : backend::Application(applicationTitle) {
    }

    Application::~Application() {
    }

    void Application::UpdateWindow() {
        glfwPollEvents();

        if (glfwWindowShouldClose(m_glfwWindow)) {
            OnEvent(EventRequestQuit());
        }

        if (m_vkSwapChainrebuild) {
            int width, height;
            glfwGetFramebufferSize(m_glfwWindow, &width, &height);
            if (width > 0 && height > 0) {
                assert(false && "TODO");
            }
        }
    }

    bool Application::CreateWindow() {
        if (!glfwInit()) {
            return false;
        }

        if (!glfwVulkanSupported()) {
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_glfwWindow = glfwCreateWindow(m_windowWidth, m_windowHeight, m_applicationTitle.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_glfwWindow, this);

        if (m_windowPos.x != -1 && m_windowPos.y != -1) {
            glfwSetWindowPos(m_glfwWindow, m_windowPos.x, m_windowPos.y);
        }

        glfwSetWindowPosCallback(m_glfwWindow, [](GLFWwindow* window, int xpos, int ypos) {
            Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            app->m_windowMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) == GLFW_TRUE;
            if (!app->m_windowMaximized) {
                app->m_windowPos.x = xpos;
                app->m_windowPos.y = ypos;
            }
        });

        glfwSetWindowSizeCallback(m_glfwWindow, [](GLFWwindow* window, int width, int height) {
            Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            app->m_windowMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) == GLFW_TRUE;
            if (!app->m_windowMaximized) {
                app->m_windowWidth = width;
                app->m_windowHeight = height;
            }
            app->OnResize();
        });

        glfwSetKeyCallback(m_glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            if (auto const translatedEvent = FromGLFW(key, scancode, action, mods)) {
                app->OnEvent(*translatedEvent);
            }
        });

        if (m_windowMaximized) {
            glfwMaximizeWindow(m_glfwWindow);
        }

        m_context = std::make_unique<Context>();

        VkSurfaceKHR vkSurface;
        CHECK_VK_RESULT(glfwCreateWindowSurface(m_context->m_vkInstance, m_glfwWindow, nullptr, &vkSurface));
        m_customVkSurface = vkSurface;

        int width, height;
        glfwGetFramebufferSize(m_glfwWindow, &width, &height);

        m_graphics = std::make_unique<Graphics>(*m_context, m_customVkSurface, m_windowWidth, m_windowHeight);
        m_vulkanGraphics = static_cast<Graphics*>(m_graphics.get());
        m_imageFactory = std::make_unique<ImageFactory>(*m_context, static_cast<Graphics&>(*m_vulkanGraphics));
        m_fontFactory = std::make_unique<FontFactory>(*m_context, *m_vulkanGraphics);
        m_uiRenderer = std::make_unique<UIRenderer>(*m_vulkanGraphics);
        auto uiContext = std::make_shared<moth_ui::Context>(m_imageFactory.get(), m_fontFactory.get(), m_uiRenderer.get());
        moth_ui::Context::SetCurrentContext(uiContext);

        SubImage::s_graphicsContext = m_vulkanGraphics;

        ImGuiInit();
        SetupLayers();

        return true;
    }

    void Application::SetWindowTitle(std::string const& title) {
        m_applicationTitle = title;
        glfwSetWindowTitle(m_glfwWindow, m_applicationTitle.c_str());
    }

    void Application::Draw() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        m_vulkanGraphics->Begin();
        m_layerStack->Draw();
        ImGui::Render();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        if (ImDrawData* drawData = ImGui::GetDrawData()) {
            ImGui_ImplVulkan_RenderDrawData(drawData, m_vulkanGraphics->GetCurrentCommandBuffer()->GetVkCommandBuffer());
        }
        m_vulkanGraphics->End();
    }

    void Application::DestroyWindow() {
        vkDeviceWaitIdle(m_context->m_vkDevice);
        m_layerStack.reset(); // force layers to cleanup before we destroy all the devices.
        m_uiRenderer.reset();
        m_fontFactory.reset();
        m_imageFactory.reset();
        m_graphics.reset();
        m_windowMaximized = glfwGetWindowAttrib(m_glfwWindow, GLFW_MAXIMIZED) == GLFW_TRUE;
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroySurfaceKHR(m_context->m_vkInstance, m_customVkSurface, nullptr);
        glfwDestroyWindow(m_glfwWindow);
        glfwTerminate();
        m_context.reset();
    }

    void Application::ImGuiInit() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
        ImGui::GetIO().IniFilename = m_imguiSettingsPath.c_str();

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplGlfw_InitForVulkan(m_glfwWindow, true);
        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = m_context->m_vkInstance;
        initInfo.PhysicalDevice = m_context->m_vkPhysicalDevice;
        initInfo.Device = m_context->m_vkDevice;
        initInfo.QueueFamily = m_context->m_vkQueueFamily;
        initInfo.Queue = m_context->m_vkQueue;
        initInfo.DescriptorPool = m_context->m_vkDescriptorPool;
        initInfo.Subpass = 0;
        initInfo.MinImageCount = m_vulkanGraphics->GetSwapchain().GetImageCount();
        initInfo.ImageCount = m_vulkanGraphics->GetSwapchain().GetImageCount();
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = checkVkResult;
        ImGui_ImplVulkan_Init(&initInfo, m_vulkanGraphics->GetRenderPass().GetRenderPass());

        // create the font texture
        {
            VkCommandPool commandPool = m_context->m_vkCommandPool;
            CHECK_VK_RESULT(vkResetCommandPool(m_context->m_vkDevice, commandPool, 0));
            VkCommandBuffer commandBuffer = m_context->beginSingleTimeCommands();
            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
            m_context->endSingleTimeCommands(commandBuffer);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void Application::OnResize() {
        m_vulkanGraphics->OnResize(m_customVkSurface, m_windowWidth, m_windowHeight);
    }
}
