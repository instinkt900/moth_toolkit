#pragma once

#include "application.h"

#include <SDL.h>

namespace backend::sdl {
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
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;
    };
}
