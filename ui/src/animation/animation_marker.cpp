#include "common.h"
#include "moth_ui/animation/animation_marker.h"

namespace moth_ui {
    void to_json(nlohmann::json& json, AnimationMarker const& marker) {
        json["frame"] = marker.frame;
        json["name"] = marker.name;
    }

    void from_json(nlohmann::json const& json, AnimationMarker& marker) {
        marker.frame = json.value("frame", 0);
        marker.name = json.value("name", std::string{});
    }
}
