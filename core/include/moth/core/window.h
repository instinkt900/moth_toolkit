#pragma once

#include "moth/core/event_emitter.h"
#include "moth/core/event_window.h"
#include "moth/core/vector.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace moth::core {
    /// @brief Base class for a native window: title, size, position, and events.
    ///
    /// Owns no graphics or UI state — rendering and moth_ui integration are
    /// layered on top by gfx/bridge subclasses. Subclasses implement the
    /// platform-specific update / frame / title behaviour.
    class Window : public EventEmitter, public IEventListener {
    public:
        /// @param title Initial title bar text.
        /// @param width Initial width in pixels.
        /// @param height Initial height in pixels.
        Window(std::string_view title, int width, int height)
            : m_title(title)
            , m_windowWidth(width)
            , m_windowHeight(height) {}

        virtual ~Window() = default;

        /// @brief Poll events and advance by @p ticks milliseconds.
        virtual void Update(uint32_t ticks) = 0;

        /// @brief Begin rendering one frame.
        virtual void BeginFrame() = 0;

        /// @brief Finish rendering the frame begun by @c BeginFrame.
        virtual void EndFrame() = 0;

        /// @brief Update the window title bar text.
        virtual void SetWindowTitle(std::string_view title) = 0;

        /// @brief Returns @c true if the window is currently maximized.
        virtual bool IsMaximized() const { return m_windowMaximized; }

        /// @brief Returns the current window position in screen coordinates.
        virtual IntVec2 const& GetPosition() const { return m_windowPos; }

        /// @brief Returns the current window width in pixels.
        virtual int GetWidth() const { return m_windowWidth; }

        /// @brief Returns the current window height in pixels.
        virtual int GetHeight() const { return m_windowHeight; }

        /// @brief Returns the logical render size used to map input to logical
        ///        coordinates (accounting for letterboxing).
        ///
        /// Defaults to the window size (no letterbox). Graphics/UI subclasses
        /// override this to report their logical resolution.
        virtual IntVec2 GetRenderSize() const { return { m_windowWidth, m_windowHeight }; }

        // IEventListener — by default, rebroadcast to external listeners.
        bool OnEvent(Event const& event) override { return EmitEvent(event); }

    protected:
        std::string m_title;
        int m_windowWidth = 0;
        int m_windowHeight = 0;
        IntVec2 m_windowPos = { -1, -1 };
        bool m_windowMaximized = false;
    };
}
