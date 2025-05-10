#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/events/event.h"

#include <memory>
#include <string>

namespace moth_ui {
    class EventAnimation : public Event {
    public:
        EventAnimation(Node* node, std::string const& name)
            : Event(GetStaticType())
            , m_node(node)
            , m_name(name) {}

        EventAnimation(EventAnimation const&) = default;
        EventAnimation(EventAnimation&&) = default;
        EventAnimation& operator=(EventAnimation const&) = default;
        EventAnimation& operator=(EventAnimation&&) = default;
        ~EventAnimation() override = default;

        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION; }

        Node* GetNode() const { return m_node; }
        std::string const& GetName() const { return m_name; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventAnimation>(m_node, m_name);
        }

    private:
        Node* m_node = nullptr;
        std::string m_name;
    };

    class EventAnimationStarted : public Event {
    public:
        EventAnimationStarted(Node* node, std::string const& clipName)
            : Event(GetStaticType())
            , m_node(node)
            , m_clipName(clipName) {}

        EventAnimationStarted(EventAnimationStarted const&) = default;
        EventAnimationStarted(EventAnimationStarted&&) = default;
        EventAnimationStarted& operator=(EventAnimationStarted const&) = default;
        EventAnimationStarted& operator=(EventAnimationStarted&&) = default;
        ~EventAnimationStarted() override = default;

        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION_STARTED; }

        Node* GetNode() const { return m_node; }
        std::string const& GetClipName() const { return m_clipName; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventAnimationStarted>(m_node, m_clipName);
        }

    private:
        Node* m_node = nullptr;
        std::string m_clipName;
    };

    class EventAnimationStopped : public Event {
    public:
        EventAnimationStopped(Node* node, std::string const& clipName)
            : Event(GetStaticType())
            , m_node(node)
            , m_clipName(clipName) {}

        EventAnimationStopped(EventAnimationStopped const&) = default;
        EventAnimationStopped(EventAnimationStopped&&) = default;
        EventAnimationStopped& operator=(EventAnimationStopped const&) = default;
        EventAnimationStopped& operator=(EventAnimationStopped&&) = default;
        ~EventAnimationStopped() override = default;

        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION_STOPPED; }

        Node* GetNode() const { return m_node; }
        std::string const& GetClipName() const { return m_clipName; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventAnimationStopped>(m_node, m_clipName);
        }

    private:
        Node* m_node = nullptr;
        std::string m_clipName;
    };
}
