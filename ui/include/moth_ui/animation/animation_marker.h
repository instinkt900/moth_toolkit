#pragma once

#include "moth_ui/moth_ui_fwd.h"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>

namespace moth_ui {
    /**
     * @brief A named marker placed at a specific frame on a group's timeline.
     *
     * When the animation controller passes an AnimationMarker's frame, it fires
     * an EventAnimation with the corresponding name.
     */
    class AnimationMarker {
    public:
        /**
         * @brief Constructs an animation marker.
         * @param frame Frame index at which the event fires.
         * @param name  Name dispatched with the EventAnimation.
         */
        AnimationMarker(int frame, std::string_view name)
            : frame(frame)
            , name(name) {}

        int frame = 0;   ///< Frame at which this event fires.
        std::string name; ///< Name sent with the EventAnimation.

        bool operator==(AnimationMarker const& other) const {
            return frame == other.frame && name == other.name;
        }

        bool operator!=(AnimationMarker const& other) const {
            return !(*this == other);
        }

        /// @brief Serializes this event to JSON.
        friend void to_json(nlohmann::json& json, AnimationMarker const& event);
        /// @brief Deserializes an event from JSON.
        friend void from_json(nlohmann::json const& json, AnimationMarker& event);

        AnimationMarker() = default;
        AnimationMarker(AnimationMarker const&) = default;
        AnimationMarker(AnimationMarker&&) = default;
        AnimationMarker& operator=(AnimationMarker const&) = default;
        AnimationMarker& operator=(AnimationMarker&&) = default;
        ~AnimationMarker() = default;
    };
}
