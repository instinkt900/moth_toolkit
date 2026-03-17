#pragma once

#include "moth_ui/moth_ui_fwd.h"
#include "moth_ui/events/event.h"

#include <memory>
#include <string>

namespace moth_ui {
    /**
     * @brief Event fired when an animation marker frame is reached during playback.
     */
    class EventAnimation : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param node The node whose animation triggered the marker.
         * @param name Name of the animation marker that fired.
         */
        EventAnimation(Node* node, std::string const& name)
            : Event(GetStaticType())
            , m_node(node)
            , m_name(name) {}

        EventAnimation(EventAnimation const&) = default;
        EventAnimation(EventAnimation&&) = default;
        EventAnimation& operator=(EventAnimation const&) = default;
        EventAnimation& operator=(EventAnimation&&) = default;
        ~EventAnimation() override = default;

        /// @brief Returns the static type code for EventAnimation.
        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION; }

        /// @brief Returns the node whose animation fired the marker.
        Node* GetNode() const { return m_node; }

        /// @brief Returns the name of the animation marker that fired.
        std::string const& GetName() const { return m_name; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventAnimation>(m_node, m_name);
        }

    private:
        Node* m_node = nullptr;
        std::string m_name;
    };

    /**
     * @brief Event fired when an animation clip begins playing.
     */
    class EventAnimationStarted : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param node     The node on which the clip started.
         * @param clipName Name of the clip that started.
         */
        EventAnimationStarted(Node* node, std::string const& clipName)
            : Event(GetStaticType())
            , m_node(node)
            , m_clipName(clipName) {}

        EventAnimationStarted(EventAnimationStarted const&) = default;
        EventAnimationStarted(EventAnimationStarted&&) = default;
        EventAnimationStarted& operator=(EventAnimationStarted const&) = default;
        EventAnimationStarted& operator=(EventAnimationStarted&&) = default;
        ~EventAnimationStarted() override = default;

        /// @brief Returns the static type code for EventAnimationStarted.
        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION_STARTED; }

        /// @brief Returns the node on which the clip started.
        Node* GetNode() const { return m_node; }

        /// @brief Returns the name of the clip that started.
        std::string const& GetClipName() const { return m_clipName; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventAnimationStarted>(m_node, m_clipName);
        }

    private:
        Node* m_node = nullptr;
        std::string m_clipName;
    };

    /**
     * @brief Event fired when an animation clip finishes or is stopped.
     */
    class EventAnimationStopped : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param node     The node on which the clip stopped.
         * @param clipName Name of the clip that stopped.
         */
        EventAnimationStopped(Node* node, std::string const& clipName)
            : Event(GetStaticType())
            , m_node(node)
            , m_clipName(clipName) {}

        EventAnimationStopped(EventAnimationStopped const&) = default;
        EventAnimationStopped(EventAnimationStopped&&) = default;
        EventAnimationStopped& operator=(EventAnimationStopped const&) = default;
        EventAnimationStopped& operator=(EventAnimationStopped&&) = default;
        ~EventAnimationStopped() override = default;

        /// @brief Returns the static type code for EventAnimationStopped.
        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION_STOPPED; }

        /// @brief Returns the node on which the clip stopped.
        Node* GetNode() const { return m_node; }

        /// @brief Returns the name of the clip that stopped.
        std::string const& GetClipName() const { return m_clipName; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventAnimationStopped>(m_node, m_clipName);
        }

    private:
        Node* m_node = nullptr;
        std::string m_clipName;
    };
}
