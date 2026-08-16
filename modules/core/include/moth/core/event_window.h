#pragma once

#include "moth/core/event_types.h"

#include <moth/core/event.h>

#include <memory>

namespace moth::core {
    /// @brief Fired when the render device is lost and recreated (e.g. GPU reset).
    class EventRenderDeviceReset : public moth::core::Event {
    public:
        EventRenderDeviceReset()
            : Event(GetStaticType()) {}
        ~EventRenderDeviceReset() override = default;

        static constexpr int GetStaticType() { return EVENTTYPE_RENDERDEVICERESET; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventRenderDeviceReset>();
        }
    };

    /// @brief Fired when a render target is invalidated and must be recreated.
    class EventRenderTargetReset : public moth::core::Event {
    public:
        EventRenderTargetReset()
            : Event(GetStaticType()) {}
        ~EventRenderTargetReset() override = default;

        static constexpr int GetStaticType() { return EVENTTYPE_RENDERTARGETRESET; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventRenderTargetReset>();
        }
    };

    /// @brief Fired when a window is resized.
    class EventWindowSize : public moth::core::Event {
    public:
        /// @param width New window width in pixels.
        /// @param height New window height in pixels.
        EventWindowSize(int width, int height)
            : Event(GetStaticType())
            , m_width(width)
            , m_height(height) {}
        ~EventWindowSize() override = default;

        static constexpr int GetStaticType() { return EVENTTYPE_WINDOWSIZE; }

        /// @brief Returns the new window width in pixels.
        int GetWidth() const { return m_width; }

        /// @brief Returns the new window height in pixels.
        int GetHeight() const { return m_height; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventWindowSize>(m_width, m_height);
        }

    private:
        int m_width = 0;
        int m_height = 0;
    };

    /// @brief Fired when the user or system requests the application to close
    ///        (e.g. clicking the window close button).
    ///
    /// Handlers may ignore this event to veto the close. If no handler vetoes it,
    /// @c Application will stop the main loop.
    class EventRequestQuit : public moth::core::Event {
    public:
        EventRequestQuit()
            : Event(GetStaticType()) {}
        ~EventRequestQuit() override = default;

        static constexpr int GetStaticType() { return EVENTTYPE_REQUEST_QUIT; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventRequestQuit>();
        }
    };

    /// @brief Fired to unconditionally terminate the application.
    class EventQuit : public moth::core::Event {
    public:
        EventQuit()
            : Event(GetStaticType()) {}
        ~EventQuit() override = default;

        static constexpr int GetStaticType() { return EVENTTYPE_QUIT; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventQuit>();
        }
    };
}
