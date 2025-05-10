#include "common.h"
#include "moth_ui/animation/animation_event.h"

namespace moth_ui {
    void to_json(nlohmann::json& json, AnimationEvent const& event) {
        json["frame"] = event.m_frame;
        json["name"] = event.m_name;
    }

    void from_json(nlohmann::json const& json, AnimationEvent& event) {
        json.at("frame").get_to(event.m_frame);
        json.at("name").get_to(event.m_name);
    }
}
