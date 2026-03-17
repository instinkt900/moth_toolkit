#pragma once

#include "moth_ui/moth_ui_fwd.h"

#include <nlohmann/json_fwd.hpp>
#include <string>

namespace moth_ui {
    /**
     * @brief A named marker placed at a specific frame on a group's timeline.
     *
     * When the animation controller passes an AnimationEvent's frame, it fires
     * an EventAnimation with the corresponding name.
     */
    class AnimationEvent {
    public:
        /**
         * @brief Constructs an animation event marker.
         * @param frame Frame index at which the event fires.
         * @param name  Name dispatched with the EventAnimation.
         */
        AnimationEvent(int frame, std::string const& name)
            : m_frame(frame)
            , m_name(name) {}

        int m_frame = 0;   ///< Frame at which this event fires.
        std::string m_name; ///< Name sent with the EventAnimation.

        bool operator==(AnimationEvent const& other) const {
            return m_frame == other.m_frame && m_name == other.m_name;
        }

        bool operator!=(AnimationEvent const& other) const {
            return !(*this == other);
        }

        friend void to_json(nlohmann::json& json, AnimationEvent const& event);
        friend void from_json(nlohmann::json const& json, AnimationEvent& event);

        AnimationEvent() = default;
        AnimationEvent(AnimationEvent const&) = default;
        AnimationEvent(AnimationEvent&&) = default;
        AnimationEvent& operator=(AnimationEvent const&) = default;
        AnimationEvent& operator=(AnimationEvent&&) = default;
        ~AnimationEvent() = default;
    };
}
