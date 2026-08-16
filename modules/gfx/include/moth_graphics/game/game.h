#pragma once

#include "moth_graphics/graphics/igraphics.h"
#include "moth_graphics/platform/glfw/glfw_platform.h"
#include "moth_graphics/platform/window.h"

#include <memory>
#include <string>

namespace moth::gfx::game {
    using moth::gfx::IGraphics;

    /**
     * @brief A single game state with Update/Draw hooks.
     *
     * Subclass to implement game logic. The owning @c Game calls @c Update then
     * @c Draw once per frame. This is a minimal "state" abstraction, distinct
     * from an entity/scene-graph model — compose it with `moth::ecs` or any
     * other structure inside your @c Draw.
     */
    class Scene {
    public:
        virtual ~Scene() = default;

        /// @brief Called once, just before the game loop begins.
        ///
        /// Load resources and initialize state here. The window and graphics
        /// are already valid, and the scene is destroyed before the window, so
        /// GPU resources created here are released in the right order.
        virtual void OnStart() {}

        /// @brief Called once, just after the game loop exits.
        ///
        /// Release non-RAII resources here (the graphics context is still valid).
        virtual void OnStop() {}

        /// @brief Per-frame logic. @p dt is the elapsed time in seconds.
        virtual void Update(float dt) {}

        /// @brief Per-frame rendering. Draw in logical pixel space.
        virtual void Draw(IGraphics& graphics) {}
    };

    /**
     * @brief A code-first game runner: window + graphics + loop, with no UI.
     *
     * The shortest path from `main` to a running game:
     * @code
     *   int main() {
     *       moth::gfx::game::Game game{ "My Game", 1280, 720 };
     *       return game.Run(std::make_unique<MyScene>());
     *   }
     * @endcode
     *
     * Owns the GLFW platform and window; the window is destroyed before the
     * platform shuts down so GPU resources are released in the right order.
     * Non-copyable.
     */
    class Game {
    public:
        /// @brief Starts the platform and creates the window. Throws
        ///        @c std::runtime_error if either fails.
        Game(std::string title, int width, int height);

        ~Game();

        Game(Game const&) = delete;
        Game& operator=(Game const&) = delete;

        /// @brief Runs the loop with @p scene until the window is closed.
        /// @returns The process exit code (0 on a clean shutdown).
        int Run(std::unique_ptr<Scene> scene);

        /// @brief Requests the loop to exit at the end of the current frame.
        void Quit();

        /// @brief Returns the window (input, events, title, …).
        moth::gfx::platform::Window& GetWindow() { return *m_window; }

        /// @brief Returns the graphics interface for drawing.
        IGraphics& GetGraphics() { return m_window->GetGraphics(); }

    private:
        moth::gfx::platform::glfw::Platform m_platform;
        std::unique_ptr<moth::gfx::platform::Window> m_window;
        bool m_running = false;
    };
}
