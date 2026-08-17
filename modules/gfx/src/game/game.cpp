#include "moth/graphics/game/game.h"

#include <moth/core/event_window.h>

#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace moth::gfx::game {
    Game::Game(std::string title, int width, int height) {
        if (!m_platform.Startup()) {
            throw std::runtime_error("moth::gfx::game::Game: failed to start the platform layer");
        }
        m_window = m_platform.CreateWindow(title, width, height);
        if (!m_window) {
            throw std::runtime_error("moth::gfx::game::Game: failed to create the window");
        }
        m_window->GetGraphics().SetLogicalSize({ width, height });
    }

    Game::~Game() {
        // Destroy the window (and its GPU resources) before the platform tears
        // down the Vulkan instance/device.
        m_window.reset();
        m_platform.Shutdown();
    }

    int Game::Run(std::unique_ptr<Scene> scene) {
        if (!m_window || !scene) {
            return 1;
        }

        m_running = true;

        m_window->AddEventListener([this](moth::core::Event const& event) {
            if (moth::core::event_cast<moth::core::EventRequestQuit>(event) != nullptr) {
                m_running = false;
                return true;
            }
            return false;
        });

        scene->OnStart();

        auto lastTime = std::chrono::steady_clock::now();
        while (m_running) {
            auto const now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - lastTime).count();
            lastTime = now;
            dt = std::clamp(dt, 0.0f, 0.1f);

            m_window->Update(16);
            scene->Update(dt);

            m_window->BeginFrame();
            scene->Draw(m_window->GetGraphics());
            m_window->EndFrame();
        }

        scene->OnStop();

        return 0;
    }

    void Game::Quit() {
        m_running = false;
    }
}
