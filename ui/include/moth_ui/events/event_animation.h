#pragma once

#include "event.h"
#include "moth_ui/animation_track.h"

namespace moth_ui {
    class EventAnimation : public Event {
    public:
        EventAnimation(Node* node, AnimationTrack::Target target, std::string const& name)
            : Event(GetStaticType())
            , m_node(node)
            , m_target(target)
            , m_name(name) {}
        virtual ~EventAnimation() {}

        static constexpr int GetStaticType() { return EVENTTYPE_ANIMATION; }

        auto GetNode() const { return m_node; }
        auto GetTarget() const { return m_target; }
        auto const& GetName() const { return m_name; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventAnimation>(m_node, m_target, m_name);
        }

    private:
        Node* m_node = nullptr;
        AnimationTrack::Target m_target = AnimationTrack::Target::Unknown;
        std::string m_name;
    };
}
