#include "common.h"
#include "sdl/sdl_app.h"

#include "sdl/sdl_ui_renderer.h"
#include "sdl/sdl_events.h"
#include "sdl/sdl_image_factory.h"
#include "sdl/sdl_font_factory.h"
#include "sdl/sdl_graphics.h"

#include "moth_ui/context.h"

#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_sdlrenderer.h>

namespace backend::sdl {
    Application::Application(std::string const& applicationTitle)
        : backend::Application(applicationTitle) {
    }

    Application::~Application() {
    }

    void Application::UpdateWindow() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (ImGui::GetIO().WantCaptureKeyboard && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
                continue;
            }
            if (auto const translatedEvent = FromSDL(event)) {
                OnEvent(*translatedEvent);
            }
        }
    }

    bool Application::CreateWindow() {
        if (0 > SDL_Init(SDL_INIT_EVERYTHING)) {
            return false;
        }

        if (nullptr == (m_window = SDL_CreateWindow(m_applicationTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_windowWidth, m_windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE))) {
            return false;
        }

        if (nullptr == (m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))) {
            return false;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        if (!m_imguiSettingsPath.empty()) {
            io.IniFilename = m_imguiSettingsPath.c_str();
        }
        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_renderer);
        ImGui_ImplSDLRenderer_Init(m_renderer);

        int const imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
        if (imgFlags != IMG_Init(imgFlags)) {
            return false;
        }

        m_graphics = std::make_unique<backend::sdl::SDLGraphics>(m_renderer);
        m_imageFactory = std::make_unique<ImageFactory>(*m_renderer);
        m_fontFactory = std::make_unique<FontFactory>(*m_renderer);
        m_uiRenderer = std::make_unique<UIRenderer>(*m_renderer);
        m_uiContext = std::make_shared<moth_ui::Context>(m_imageFactory.get(), m_fontFactory.get(), m_uiRenderer.get());

        SetupLayers();

        return true;
    }

    void Application::SetWindowTitle(std::string const& title) {
        m_applicationTitle = title;
        SDL_SetWindowTitle(m_window, m_applicationTitle.c_str());
    }

    void Application::Draw() {
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame(m_window);
        ImGui::NewFrame();

        m_layerStack->Draw();

        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(m_renderer);
    }

    void Application::DestroyWindow() {
        ImGui_ImplSDLRenderer_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        IMG_Quit();
        SDL_Quit();
    }
}
