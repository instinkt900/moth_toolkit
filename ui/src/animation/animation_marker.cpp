#include "common.h"
#include "moth_ui/animation/animation_marker.h"

namespace moth_ui {
    void to_json(nlohmann::json& json, AnimationMarker const& event) {
        json["frame"] = event.frame;
        json["name"] = event.name;
    }

    void from_json(nlohmann::json const& json, AnimationMarker& event) {
        event.frame = json.value("frame", 0);
        event.name = json.value("name", std::string{});
    }
}
