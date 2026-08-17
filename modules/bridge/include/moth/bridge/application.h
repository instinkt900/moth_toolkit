#pragma once

#include "moth/bridge/ui_window.h"
#include "moth/graphics/platform/iplatform.h"

#include <moth/core/event.h>
#include <moth/core/event_listener.h>
#include <moth/core/ticker.h>

#include <cassert>
#include <memory>
#include <string>
#include <string_view>

namespace moth::bridge {
    /// @brief A UI-driven application: platform + UiWindow + ImGui + fixed-timestep loop.
    class Application : public moth::core::Ticker, public moth::core::IEventListener {
    public:
        Application(moth::gfx::platform::IPlatform& platform, std::string_view title, int width, int height);
        ~Application() override = default;

        void Init();
        void Run();

        bool OnEvent(moth::core::Event const& event) override;

        void TickFixed(uint32_t ticks) override;
        void Tick(uint32_t ticks) override;

        /// @brief Returns the UI window, or @c nullptr before @c Init().
        UiWindow* GetUiWindow() { return m_uiWindow.get(); }

        /// @brief Returns the underlying graphics window, or @c nullptr before @c Init().
        moth::gfx::platform::Window* GetWindow() { return m_uiWindow ? &m_uiWindow->GetWindow() : nullptr; }

        /// @brief Enable or disable ImGui multi-viewport support. Must be called before Init().
        void SetImGuiViewportsEnabled(bool enabled) {
            assert(m_uiWindow == nullptr && "SetImGuiViewportsEnabled must be called before Init()");
            m_imguiViewportsEnabled = enabled;
        }

        Application(Application const&) = delete;
        Application& operator=(Application const&) = delete;
        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

    protected:
        /// @brief Override to perform setup before the window is created.
        virtual void Startup() {}

        /// @brief Override to perform setup after the window and ImGui are ready.
        virtual void PostCreateWindow() {}

        /// @brief Override to perform teardown after the main loop exits.
        virtual void Shutdown() {}

    private:
        bool OnWindowSizeEvent(moth::core::EventWindowSize const& event);
        bool OnRequestQuitEvent(moth::core::EventRequestQuit const& event);
        bool OnQuitEvent(moth::core::EventQuit const& event);

        moth::gfx::platform::IPlatform& m_platform;
        std::string m_mainWindowTitle;
        int m_mainWindowWidth;
        int m_mainWindowHeight;
        bool m_imguiViewportsEnabled = false;
        std::unique_ptr<UiWindow> m_uiWindow;
    };
}
