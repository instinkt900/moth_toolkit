#include "common.h"
#include "moth_ui/animation/animation_event.h"

namespace moth_ui {
    void to_json(nlohmann::json& json, AnimationEvent const& event) {
        json["frame"] = event.frame;
        json["name"] = event.name;
    }

    void from_json(nlohmann::json const& json, AnimationEvent& event) {
        event.frame = json.value("frame", 0);
        event.name = json.value("name", std::string{});
    }
}
