#pragma once

#include "moth_ui/events/event.h"
#include "moth_ui/events/event_listener.h"

#include <functional>

namespace moth_ui {
    /**
     * @brief Helper that routes an Event to typed handlers, stopping after the first match.
     *
     * Construct with an event reference, then call one of the Dispatch() overloads
     * for each handler.  Once a handler returns @c true the event is marked
     * handled and subsequent Dispatch() calls are no-ops.
     *
     * @note EventDispatch is non-copyable and non-movable to prevent accidental re-use.
     */
    class EventDispatch {
    public:
        /**
         * @brief Constructs a dispatcher bound to @p event.
         * @param event The event to dispatch; must outlive this object.
         */
        EventDispatch(Event const& event)
            : m_event(event) {}

        EventDispatch(EventDispatch const&) = delete;
        EventDispatch(EventDispatch&&) = delete;
        EventDispatch& operator=(EventDispatch const&) = delete;
        EventDispatch& operator=(EventDispatch&&) = delete;
        ~EventDispatch() = default;

        /// @brief Returns @c true if any handler has consumed the event.
        bool GetHandled() const { return m_handled; }

        /**
         * @brief Forwards the event to an EventListener if not already handled.
         * @param listener Listener to call; does nothing if @c nullptr.
         */
        void Dispatch(EventListener* listener) {
            if (m_handled) {
                return;
            }
            if ((listener != nullptr) && listener->OnEvent(m_event)) {
                m_handled = true;
            }
        }

        /**
         * @brief Forwards the event to a typed member function if its type matches.
         * @tparam T Object type that owns the member function.
         * @tparam E Concrete event type the member function expects.
         * @param obj  Object to invoke the member function on.
         * @param func Member function to call with the specific event type.
         */
        template <typename T, typename E>
        void Dispatch(T* obj, bool (T::*func)(E const& event)) {
            if (m_handled || obj == nullptr) {
                return;
            }
            if (auto specificEvent = event_cast<E>(m_event)) {
                if (std::invoke(func, obj, *specificEvent)) {
                    m_handled = true;
                }
            }
        }

        /**
         * @brief Forwards the event to a typed free function or lambda if its type matches.
         * @tparam T Concrete event type the callable expects.
         * @param func Callable that accepts a @c T const& and returns @c bool.
         */
        template <typename T>
        void Dispatch(std::function<bool(T const&)> const& func) {
            if (m_handled || !func) {
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
