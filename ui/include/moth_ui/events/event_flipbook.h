#pragma once

#include "moth_ui/events/event.h"
#include "moth_ui/nodes/node_flipbook.h"

namespace moth_ui {
    /**
     * @brief Event fired when a flipbook node begins or resumes playing a clip.
     */
    class EventFlipbookStarted : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param node     The flipbook node that started playing.
         * @param clipName Name of the clip that started. May be empty if @c Play()
         *                 was called while no clip was loaded — playback begins in
         *                 a blank state and no frames will advance.
         */
        EventFlipbookStarted(std::weak_ptr<NodeFlipbook> node, std::string_view clipName)
            : Event(GetStaticType())
            , m_node(node)
            , m_clipName(clipName) {}

        EventFlipbookStarted(EventFlipbookStarted const&) = default;
        EventFlipbookStarted(EventFlipbookStarted&&) = default;
        EventFlipbookStarted& operator=(EventFlipbookStarted const&) = default;
        EventFlipbookStarted& operator=(EventFlipbookStarted&&) = default;
        ~EventFlipbookStarted() override = default;

        /// @brief Returns the static type code for EventFlipbookStarted.
        static constexpr int GetStaticType() { return EVENTTYPE_FLIPBOOK_STARTED; }

        /// @brief Returns the flipbook node that started playing.
        std::weak_ptr<NodeFlipbook> GetNode() const { return m_node; }

        /// @brief Returns the name of the clip that started playing.
        std::string_view GetClipName() const { return m_clipName; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventFlipbookStarted>(m_node, m_clipName);
        }

    private:
        std::weak_ptr<NodeFlipbook> m_node;
        std::string m_clipName;
    };

    /**
     * @brief Event fired when a flipbook clip reaches its end and stops.
     *
     * Fired for both @c LoopType::Stop (freezes on last frame) and
     * @c LoopType::Reset (rewinds to first frame). Not fired for
     * @c LoopType::Loop.
     */
    class EventFlipbookStopped : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param node     The flipbook node that stopped playing.
         * @param clipName Name of the clip that stopped.
         */
        EventFlipbookStopped(std::weak_ptr<NodeFlipbook> node, std::string_view clipName)
            : Event(GetStaticType())
            , m_node(node)
            , m_clipName(clipName) {}

        EventFlipbookStopped(EventFlipbookStopped const&) = default;
        EventFlipbookStopped(EventFlipbookStopped&&) = default;
        EventFlipbookStopped& operator=(EventFlipbookStopped const&) = default;
        EventFlipbookStopped& operator=(EventFlipbookStopped&&) = default;
        ~EventFlipbookStopped() override = default;

        /// @brief Returns the static type code for EventFlipbookStopped.
        static constexpr int GetStaticType() { return EVENTTYPE_FLIPBOOK_STOPPED; }

        /// @brief Returns the flipbook node that stopped playing.
        std::weak_ptr<NodeFlipbook> GetNode() const { return m_node; }

        /// @brief Returns the name of the clip that stopped.
        std::string_view GetClipName() const { return m_clipName; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventFlipbookStopped>(m_node, m_clipName);
        }

    private:
        std::weak_ptr<NodeFlipbook> m_node;
        std::string m_clipName;
    };
}
