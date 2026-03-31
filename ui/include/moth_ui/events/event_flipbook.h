#pragma once

#include "moth_ui/events/event.h"
#include "moth_ui/nodes/node.h"

namespace moth_ui {
    /**
     * @brief Event fired when a flipbook node begins playing.
     */
    class EventFlipbookStarted : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param node The flipbook node that started playing.
         */
        EventFlipbookStarted(Node* node)
            : Event(GetStaticType())
            , m_node(node) {}

        EventFlipbookStarted(EventFlipbookStarted const&) = default;
        EventFlipbookStarted(EventFlipbookStarted&&) = default;
        EventFlipbookStarted& operator=(EventFlipbookStarted const&) = default;
        EventFlipbookStarted& operator=(EventFlipbookStarted&&) = default;
        ~EventFlipbookStarted() override = default;

        /// @brief Returns the static type code for EventFlipbookStarted.
        static constexpr int GetStaticType() { return EVENTTYPE_FLIPBOOK_STARTED; }

        /// @brief Returns the flipbook node that started playing.
        Node* GetNode() const { return m_node; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventFlipbookStarted>(m_node);
        }

    private:
        Node* m_node = nullptr;
    };

    /**
     * @brief Event fired when a flipbook node reaches its last frame and stops
     *        (only fired when loop is false).
     */
    class EventFlipbookStopped : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param node The flipbook node that stopped playing.
         */
        EventFlipbookStopped(Node* node)
            : Event(GetStaticType())
            , m_node(node) {}

        EventFlipbookStopped(EventFlipbookStopped const&) = default;
        EventFlipbookStopped(EventFlipbookStopped&&) = default;
        EventFlipbookStopped& operator=(EventFlipbookStopped const&) = default;
        EventFlipbookStopped& operator=(EventFlipbookStopped&&) = default;
        ~EventFlipbookStopped() override = default;

        /// @brief Returns the static type code for EventFlipbookStopped.
        static constexpr int GetStaticType() { return EVENTTYPE_FLIPBOOK_STOPPED; }

        /// @brief Returns the flipbook node that stopped playing.
        Node* GetNode() const { return m_node; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventFlipbookStopped>(m_node);
        }

    private:
        Node* m_node = nullptr;
    };
}
