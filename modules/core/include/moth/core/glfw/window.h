#pragma once

#include <moth/core/window.h>

#include <GLFW/glfw3.h>

#include <string_view>

namespace moth::core::glfw {
    /// @brief A native GLFW window: owns the GLFW handle, callbacks, and input.
    ///
    /// Rendering and UI are layered on top by an owner (e.g. the gfx window)
    /// via the @c Listener interface. This class knows nothing about graphics
    /// or moth_ui.
    class Window : public moth::core::Window {
    public:
        /// @brief Owner-side hooks for render-size queries and event delivery.
        struct Listener {
            virtual ~Listener() = default;

            /// @brief Logical render size used to map input (letterboxing).
            virtual IntVec2 GetRenderSize() const = 0;

            /// @brief Delivers a translated input/window event.
            /// @return @c true if the event was consumed.
            virtual bool OnEvent(Event const& event) = 0;

            /// @brief Called when the window framebuffer resizes.
            virtual void OnResize(int width, int height) = 0;
        };

        /// @param title Initial title bar text.
        /// @param width Initial width in pixels.
        /// @param height Initial height in pixels.
        Window(std::string_view title, int width, int height);
        ~Window() override;

        /// @brief Install the owner listener for input mapping and event delivery.
        void SetListener(Listener* listener) { m_listener = listener; }

        void Update(uint32_t ticks) override;
        void BeginFrame() override {}
        void EndFrame() override {}
        void SetWindowTitle(std::string_view title) override;

        /// @brief Returns the underlying GLFW window handle.
        GLFWwindow* GetGLFWWindow() const { return m_glfwWindow; }

    private:
        bool CreateWindow();
        void OnResize();

        Listener* m_listener = nullptr;
        GLFWwindow* m_glfwWindow = nullptr;
        FloatVec2 m_lastMousePos;
        bool m_haveMousePos = false;
    };
}
