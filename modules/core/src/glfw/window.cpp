#include "moth/core/glfw/window.h"
#include "moth/core/glfw/events.h"
#include "moth/core/glfw/gamepad.h"
#include "moth/core/event_window.h"
#include "moth/core/event_mouse.h"
#include "moth/core/input.h"

#include <cassert>

namespace moth::core::glfw {
    namespace {
        // Translate a raw window-pixel position (and optional delta) into
        // logical/render coordinates, accounting for the letterbox introduced
        // when window aspect differs from the render aspect.
        struct LogicalScale {
            float scaleX;
            float scaleY;
            float offsetX;
            float offsetY;
        };

        LogicalScale ComputeLogicalScale(IntVec2 windowSize, IntVec2 renderSize) {
            float const ww = static_cast<float>(windowSize.x);
            float const wh = static_cast<float>(windowSize.y);
            float const lw = static_cast<float>(renderSize.x);
            float const lh = static_cast<float>(renderSize.y);
            if (ww <= 0.0f || wh <= 0.0f || lw <= 0.0f || lh <= 0.0f) {
                return { 1.0f, 1.0f, 0.0f, 0.0f };
            }
            float const logicalAspect = lw / lh;
            float const windowAspect = ww / wh;
            float fitWidth = ww;
            float fitHeight = wh;
            if (windowAspect > logicalAspect) {
                fitWidth = wh * logicalAspect;
            } else {
                fitHeight = ww / logicalAspect;
            }
            float const offsetX = (ww - fitWidth) * 0.5f;
            float const offsetY = (wh - fitHeight) * 0.5f;
            return { lw / fitWidth, lh / fitHeight, offsetX, offsetY };
        }

        IntVec2 ToLogicalPos(IntVec2 windowSize, IntVec2 renderSize, FloatVec2 const& windowPos) {
            auto const s = ComputeLogicalScale(windowSize, renderSize);
            return IntVec2{
                static_cast<int>((windowPos.x - s.offsetX) * s.scaleX),
                static_cast<int>((windowPos.y - s.offsetY) * s.scaleY),
            };
        }

        FloatVec2 ToLogicalDelta(IntVec2 windowSize, IntVec2 renderSize, FloatVec2 const& windowDelta) {
            auto const s = ComputeLogicalScale(windowSize, renderSize);
            return FloatVec2{ windowDelta.x * s.scaleX, windowDelta.y * s.scaleY };
        }
    }

    Window::Window(std::string_view title, int width, int height)
        : moth::core::Window(title, width, height) {
        CreateWindow();
    }

    Window::~Window() {
        if (m_glfwWindow != nullptr) {
            glfwDestroyWindow(m_glfwWindow);
            m_glfwWindow = nullptr;
        }
    }

    void Window::Update(uint32_t /*ticks*/) {
        Input::Get().BeginFrame();
        glfwPollEvents();
        glfw::PollGamepads();

        if (glfwWindowShouldClose(m_glfwWindow) != 0) {
            glfwSetWindowShouldClose(m_glfwWindow, 0);
            EventRequestQuit const event;
            if (m_listener) {
                m_listener->OnEvent(event);
            } else {
                OnEvent(event);
            }
        }

        m_windowMaximized = glfwGetWindowAttrib(m_glfwWindow, GLFW_MAXIMIZED) == GLFW_TRUE;
    }

    void Window::SetWindowTitle(std::string_view title) {
        m_title = title;
        glfwSetWindowTitle(m_glfwWindow, m_title.c_str());
    }

    bool Window::CreateWindow() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_glfwWindow = glfwCreateWindow(m_windowWidth, m_windowHeight, m_title.c_str(), nullptr, nullptr);
        if (m_glfwWindow == nullptr) {
            return false;
        }
        glfwSetWindowUserPointer(m_glfwWindow, this);

        if (m_windowPos.x != -1 && m_windowPos.y != -1) {
            glfwSetWindowPos(m_glfwWindow, m_windowPos.x, m_windowPos.y);
        }

        auto const windowSize = [this]() { return IntVec2{ m_windowWidth, m_windowHeight }; };
        auto const renderSize = [this]() { return m_listener ? m_listener->GetRenderSize() : GetRenderSize(); };
        auto const deliver = [this](Event const& event) {
            if (m_listener) {
                m_listener->OnEvent(event);
            } else {
                OnEvent(event);
            }
        };

        glfwSetWindowPosCallback(m_glfwWindow, [](GLFWwindow* window, int xpos, int ypos) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (self == nullptr) {
                return;
            }
            self->m_windowMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) == GLFW_TRUE;
            if (!self->m_windowMaximized) {
                self->m_windowPos.x = xpos;
                self->m_windowPos.y = ypos;
            }
        });

        glfwSetWindowSizeCallback(m_glfwWindow, [](GLFWwindow* window, int width, int height) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (self == nullptr) {
                return;
            }
            self->m_windowMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) == GLFW_TRUE;
            if (!self->m_windowMaximized) {
                self->m_windowWidth = width;
                self->m_windowHeight = height;
            }

            int fbWidth = 0;
            int fbHeight = 0;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
            if (fbWidth > 0 && fbHeight > 0 && self->m_listener != nullptr) {
                self->m_listener->OnResize(fbWidth, fbHeight);
            }

            EventWindowSize const translatedEvent{ width, height };
            if (self->m_listener) {
                self->m_listener->OnEvent(translatedEvent);
            } else {
                self->OnEvent(translatedEvent);
            }
        });

        glfwSetKeyCallback(m_glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (self == nullptr) {
                return;
            }
            if (auto const translatedEvent = FromGLFW(key, scancode, action, mods)) {
                Input::Get().ProcessEvent(*translatedEvent);
                if (self->m_listener) {
                    self->m_listener->OnEvent(*translatedEvent);
                } else {
                    self->OnEvent(*translatedEvent);
                }
            }
        });

        glfwSetCursorPosCallback(m_glfwWindow, [](GLFWwindow* window, double xpos, double ypos) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (self == nullptr) {
                return;
            }
            auto const newMousePos = FloatVec2{ xpos, ypos };
            FloatVec2 windowDelta{ 0, 0 };
            if (self->m_haveMousePos) {
                windowDelta = newMousePos - self->m_lastMousePos;
            }
            self->m_lastMousePos = newMousePos;
            self->m_haveMousePos = true;
            auto const windowSize = IntVec2{ self->m_windowWidth, self->m_windowHeight };
            auto const renderSize = self->m_listener ? self->m_listener->GetRenderSize() : self->GetRenderSize();
            auto const logicalPos = ToLogicalPos(windowSize, renderSize, newMousePos);
            auto const logicalDelta = ToLogicalDelta(windowSize, renderSize, windowDelta);
            EventMouseMove const translatedEvent{ logicalPos, logicalDelta };
            Input::Get().ProcessEvent(translatedEvent);
            if (self->m_listener) {
                self->m_listener->OnEvent(translatedEvent);
            } else {
                self->OnEvent(translatedEvent);
            }
        });

        glfwSetMouseButtonCallback(m_glfwWindow, [](GLFWwindow* window, int button, int action, int mods) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (self == nullptr) {
                return;
            }
            double cursorX = 0.0;
            double cursorY = 0.0;
            glfwGetCursorPos(window, &cursorX, &cursorY);
            self->m_lastMousePos = FloatVec2{ cursorX, cursorY };
            self->m_haveMousePos = true;
            auto const windowSize = IntVec2{ self->m_windowWidth, self->m_windowHeight };
            auto const renderSize = self->m_listener ? self->m_listener->GetRenderSize() : self->GetRenderSize();
            auto const logicalPos = ToLogicalPos(windowSize, renderSize, self->m_lastMousePos);
            if (auto const translatedEvent = FromGLFW(button, action, mods, logicalPos)) {
                Input::Get().ProcessEvent(*translatedEvent);
                if (self->m_listener) {
                    self->m_listener->OnEvent(*translatedEvent);
                } else {
                    self->OnEvent(*translatedEvent);
                }
            }
        });

        glfwSetScrollCallback(m_glfwWindow, [](GLFWwindow* window, double xoffset, double yoffset) {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (self == nullptr) {
                return;
            }
            double cursorX = 0.0;
            double cursorY = 0.0;
            glfwGetCursorPos(window, &cursorX, &cursorY);
            self->m_lastMousePos = FloatVec2{ cursorX, cursorY };
            self->m_haveMousePos = true;
            auto const windowSize = IntVec2{ self->m_windowWidth, self->m_windowHeight };
            auto const renderSize = self->m_listener ? self->m_listener->GetRenderSize() : self->GetRenderSize();
            auto const logicalPos = ToLogicalPos(windowSize, renderSize, self->m_lastMousePos);
            EventMouseWheel const translatedEvent{
                IntVec2{ static_cast<int>(xoffset), static_cast<int>(yoffset) },
                logicalPos };
            Input::Get().ProcessEvent(translatedEvent);
            if (self->m_listener) {
                self->m_listener->OnEvent(translatedEvent);
            } else {
                self->OnEvent(translatedEvent);
            }
        });

        if (m_windowMaximized) {
            glfwMaximizeWindow(m_glfwWindow);
        }

        return true;
    }
}
