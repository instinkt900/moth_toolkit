#pragma once

#include <memory>

namespace moth_ui {
    /**
     * @brief Integer type codes that identify the concrete event class.
     *
     * User-defined event types should use values starting at one of the
     * @c EVENTTYPE_USER* ranges.
     */
    enum EventType : int {
        EVENTTYPE_KEY = 0,                ///< EventKey.
        EVENTTYPE_MOUSE_DOWN = 1,         ///< EventMouseDown.
        EVENTTYPE_MOUSE_UP = 2,           ///< EventMouseUp.
        EVENTTYPE_MOUSE_MOVE = 3,         ///< EventMouseMove.
        EVENTTYPE_MOUSE_WHEEL = 4,        ///< EventMouseWheel.
        EVENTTYPE_ANIMATION = 5,          ///< EventAnimation (animation marker fired).
        EVENTTYPE_ANIMATION_STARTED = 6,  ///< EventAnimationStarted.
        EVENTTYPE_ANIMATION_STOPPED = 7,  ///< EventAnimationStopped.
        EVENTTYPE_FLIPBOOK_STARTED = 8,   ///< EventFlipbookStarted.
        EVENTTYPE_FLIPBOOK_STOPPED = 9,   ///< EventFlipbookStopped.

        EVENTTYPE_USER0 = 1000,           ///< First user-defined event range.
        EVENTTYPE_USER1 = 2000,           ///< Second user-defined event range.
        EVENTTYPE_USER2 = 3000,           ///< Third user-defined event range.
    };

    /**
     * @brief Polymorphic base class for all events in the UI system.
     *
     * Each concrete event carries a type code that identifies its class,
     * enabling type-safe downcasting via event_cast<T>().
     */
    class Event {
    public:
        /**
         * @brief Constructs an event with the given type code.
         * @param type An EventType (or user-defined) value.
         */
        Event(int type)
            : m_type(type) {}

        Event(Event const&) = default;
        Event(Event&&) = default;
        Event& operator=(Event const&) = default;
        Event& operator=(Event&&) = default;
        virtual ~Event() = default;

        /// @brief Returns the integer type code of this event.
        int GetType() const { return m_type; }

        /// @brief Creates a heap-allocated copy of this event.
        virtual std::unique_ptr<Event> Clone() const = 0;

    protected:
        int m_type;
    };

    /**
     * @brief Type-safe downcast from Event to a concrete event type.
     *
     * @tparam T Concrete event type that exposes @c GetStaticType().
     * @param event Event to cast.
     * @return Pointer to @p T if the type matches, otherwise @c nullptr.
     */
    template <typename T>
    T const* event_cast(Event const& event) {
        if (event.GetType() == T::GetStaticType()) {
            return static_cast<T const*>(&event);
        }
        return nullptr;
    }
}
