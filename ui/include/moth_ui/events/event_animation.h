#pragma once

#include "event.h"
#include "moth_ui/node.h"
#include "moth_ui/animation_track.h"

namespace moth_ui {
    class EventAnimation : public Event {
    public:
        EventAnimation(Node* node, std::string const& name)
            : Event(GetStaticType())
            , m_node(node)
            , m_name(name) {}
        virtual ~EventAnimation() {}

        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION; }

        auto GetNode() const { return m_node; }
        auto const& GetName() const { return m_name; }

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
        virtual ~EventAnimationStarted() {}

        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION_STARTED; }

        auto GetNode() const { return m_node; }
        auto const& GetClipName() const { return m_clipName; }

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
        virtual ~EventAnimationStopped() {}

        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION_STOPPED; }

        auto GetNode() const { return m_node; }
        auto const& GetClipName() const { return m_clipName; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventAnimationStopped>(m_node, m_clipName);
        }

    private:
        Node* m_node = nullptr;
        std::string m_clipName;
    };
}
