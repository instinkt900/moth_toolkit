#pragma once

#include "moth_ui/events/event.h"
#include "moth_ui/events/event_listener.h"

#include <functional>

namespace moth_ui {
    class EventDispatch {
    public:
        EventDispatch(Event const& event)
            : m_event(event) {}

        EventDispatch(EventDispatch const&) = delete;
        EventDispatch(EventDispatch&&) = delete;
        EventDispatch& operator=(EventDispatch const&) = delete;
        EventDispatch& operator=(EventDispatch&&) = delete;
        ~EventDispatch() = default;

        bool GetHandled() const { return m_handled; }

        void Dispatch(EventListener* listener) {
            if (m_handled) {
                return;
            }
            if ((listener != nullptr) && listener->OnEvent(m_event)) {
                m_handled = true;
            }
        }

        template <typename T, typename E>
        void Dispatch(T* obj, bool (T::*func)(E const& event)) {
            if (m_handled) {
                return;
            }
            if (auto specificEvent = event_cast<E>(m_event)) {
                if (std::invoke(func, obj, *specificEvent)) {
                    m_handled = true;
                }
            }
        }

        template <typename T>
        void Dispatch(std::function<bool(T const&)> const& func) {
            if (m_handled) {
                return;
            }
            if (auto specificEvent = event_cast<T>(m_event)) {
                if (func(*specificEvent)) {
                    m_handled = true;
                }
            }
        }

    private:
        Event const& m_event;
        bool m_handled = false;
    };
}
