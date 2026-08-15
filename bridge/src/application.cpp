#include "moth/bridge/application.h"
#include "moth_graphics/platform/imgui_context.h"

#include <moth/core/event_window.h>

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace moth::bridge {
    Application::Application(moth_graphics::platform::IPlatform& platform, std::string_view title, int width, int height)
        : m_platform(platform)
        , m_mainWindowTitle(title)
        , m_mainWindowWidth(width)
        , m_mainWindowHeight(height) {
    }

    void Application::Init() {
        spdlog::info("Application: initializing");
        Startup();
        spdlog::info("Application: creating window '{}' ({}x{})", m_mainWindowTitle, m_mainWindowWidth, m_mainWindowHeight);
        std::unique_ptr<moth_graphics::platform::Window> window;
        try {
            window = m_platform.CreateWindow(m_mainWindowTitle, m_mainWindowWidth, m_mainWindowHeight);
        } catch (std::exception const& e) {
            spdlog::error("Application: failed to create window: {}", e.what());
            throw;
        }
        m_uiWindow = std::make_unique<UiWindow>(std::move(window));
        m_uiWindow->GetWindow().AddEventListener(this);
        auto imguiContext = m_platform.CreateImGuiContext(m_uiWindow->GetWindow(), m_uiWindow->GetGraphics(), m_imguiViewportsEnabled);
        if (!imguiContext) {
            spdlog::error("Application: ImGui context initialization failed");
            throw std::runtime_error("ImGui context initialization failed");
        }
        m_uiWindow->SetImGuiContext(std::move(imguiContext));
        PostCreateWindow();
        spdlog::info("Application: ready");
    }

    void Application::Run() {
        spdlog::info("Application: running");
        TickSync();
        spdlog::info("Application: shutting down");
        Shutdown();
        // Tear down the window before main() calls platform.Shutdown() (which
        // destroys the Vulkan instance).
        m_uiWindow.reset();
    }

    bool Application::OnEvent(moth::core::Event const& event) {
        moth::core::EventDispatch dispatch(event);
        dispatch.Dispatch(this, &Application::OnWindowSizeEvent);
        dispatch.Dispatch(this, &Application::OnRequestQuitEvent);
        dispatch.Dispatch(this, &Application::OnQuitEvent);
        return dispatch.GetHandled();
    }

    bool Application::OnWindowSizeEvent(moth::core::EventWindowSize const& /*event*/) {
        return false;
    }

    bool Application::OnRequestQuitEvent(moth::core::EventRequestQuit const& /*event*/) {
        SetRunning(false);
        return true;
    }

    bool Application::OnQuitEvent(moth::core::EventQuit const& /*event*/) {
        SetRunning(false);
        return true;
    }

    void Application::TickFixed(uint32_t ticks) {
        m_uiWindow->Update(ticks);
    }

    void Application::Tick(uint32_t /*ticks*/) {
        moth_graphics::platform::ImGuiContext* imgui = m_uiWindow->HasImGuiContext() ? &m_uiWindow->GetImGuiContext() : nullptr;
        if (imgui != nullptr) {
            imgui->NewFrame();
        }
        m_uiWindow->BeginFrame();
        m_uiWindow->GetLayerStack().Draw();
        if (imgui != nullptr) {
            imgui->Render(m_uiWindow->GetGraphics());
        }
        m_uiWindow->EndFrame();
    }
}
