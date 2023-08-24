#pragma once

#include "moth_ui/layers/layer_stack.h"
#include "igraphics_context.h"
#include "events/event.h"

#include "moth_ui/iimage_factory.h"
#include "moth_ui/font_factory.h"
#include "moth_ui/irenderer.h"

#undef CreateWindow

namespace backend {
    class Application : public moth_ui::EventListener {
    public:
        Application(std::string const& applicationTitle);
        virtual ~Application();

        virtual void SetWindowTitle(std::string const& title) = 0;

        int Run();
        void Stop() { m_running = false; }

        virtual bool OnEvent(moth_ui::Event const& event);

        backend::IGraphicsContext& GetGraphics() const { return *m_graphics; }

    protected:
        virtual bool CreateWindow() = 0;
        virtual void DestroyWindow() = 0;
        virtual void SetupLayers() = 0;
        virtual void UpdateWindow() = 0;
        virtual void Draw() = 0;

        void Update();

        bool OnWindowSizeEvent(EventWindowSize const& event);
        bool OnQuitEvent(EventQuit const& event);

        static int constexpr INIT_WINDOW_WIDTH = 1280;
        static int constexpr INIT_WINDOW_HEIGHT = 960;

        std::string m_applicationTitle;
        std::string m_imguiSettingsPath;
        moth_ui::IntVec2 m_windowPos = { -1, -1 };
        int m_windowWidth = 0;
        int m_windowHeight = 0;
        bool m_windowMaximized = false;

        bool m_running = false;
        bool m_paused = false;
        std::chrono::milliseconds m_updateTicks;
        std::chrono::time_point<std::chrono::steady_clock> m_lastUpdateTicks;

        std::unique_ptr<moth_ui::LayerStack> m_layerStack;

        std::unique_ptr<moth_ui::IImageFactory> m_imageFactory;
        std::unique_ptr<moth_ui::FontFactory> m_fontFactory;
        std::unique_ptr<moth_ui::IRenderer> m_uiRenderer;
        std::unique_ptr<backend::IGraphicsContext> m_graphics;
    };
}
