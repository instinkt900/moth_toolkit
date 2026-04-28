#include "common.h"
#include "moth_ui/animation/keyframe.h"

namespace nlohmann {
    template <>
    struct adl_serializer<std::variant<float, std::string>> {
        static void from_json(json const& json, std::variant<float, std::string>& var) {
            size_t const index = json["index"];
            if (index == 0) {
                var = static_cast<float>(json["value"]);
            } else if (index == 1) {
                var = static_cast<std::string>(json["value"]);
            }
        }

        static void to_json(json& json, std::variant<float, std::string> const& var) {
            json["index"] = var.index();
            std::visit([&](auto&& value) {
                json["value"] = value;
            },
                       var);
        }
    };
}

namespace moth_ui {
    void to_json(nlohmann::json& json, Keyframe const& keyframe) {
        json["frame"] = keyframe.frame;
        json["value"] = keyframe.value;
        json["interp"] = keyframe.interpType;
    }

    void from_json(nlohmann::json const& json, Keyframe& keyframe) {
        nlohmann::json valueJson = json.value("value", nlohmann::json{});
        if (valueJson.is_number()) {
            valueJson.get_to(keyframe.value);
        } else if (valueJson.is_object() && valueJson.contains("index")) {
            std::variant<float, std::string> oldValueType;
            valueJson.get_to(oldValueType);
            if (auto* f = std::get_if<float>(&oldValueType)) {
                keyframe.value = *f;
            }
        }

        keyframe.frame = json.value("frame", 0);
        keyframe.interpType = json.value("interp", InterpType::Linear);
    }
}
