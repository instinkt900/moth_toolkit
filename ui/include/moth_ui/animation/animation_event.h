#pragma once

#include "moth_ui/moth_ui_fwd.h"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>

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
        AnimationEvent(int frame, std::string_view name)
            : frame(frame)
            , name(name) {}

        int frame = 0;   ///< Frame at which this event fires.
        std::string name; ///< Name sent with the EventAnimation.

        bool operator==(AnimationEvent const& other) const {
            return frame == other.frame && name == other.name;
        }

        bool operator!=(AnimationEvent const& other) const {
            return !(*this == other);
        }

        /// @brief Serializes this event to JSON.
        friend void to_json(nlohmann::json& json, AnimationEvent const& event);
        /// @brief Deserializes an event from JSON.
        friend void from_json(nlohmann::json const& json, AnimationEvent& event);

        AnimationEvent() = default;
        AnimationEvent(AnimationEvent const&) = default;
        AnimationEvent(AnimationEvent&&) = default;
        AnimationEvent& operator=(AnimationEvent const&) = default;
        AnimationEvent& operator=(AnimationEvent&&) = default;
        ~AnimationEvent() = default;
    };
}
