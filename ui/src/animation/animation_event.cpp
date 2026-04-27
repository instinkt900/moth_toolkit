#include "common.h"
#include "moth_ui/animation/animation_event.h"

namespace moth_ui {
    void to_json(nlohmann::json& json, AnimationEvent const& event) {
        json["frame"] = event.m_frame;
        json["name"] = event.m_name;
    }

    void from_json(nlohmann::json const& json, AnimationEvent& event) {
        event.m_frame = json.value("frame", 0);
        event.m_name = json.value("name", std::string{});
    }
}
