#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/utils/interp.h"

#include <nlohmann/json_fwd.hpp>

namespace moth_ui {
    struct Keyframe {
        Keyframe(int frame, KeyframeValue value)
            : m_frame(frame)
            , m_value(value) {}

        int m_frame = 0;
        KeyframeValue m_value = 0;
        InterpType m_interpType = InterpType::Linear;

        friend void to_json(nlohmann::json& json, Keyframe const& keyframe);
        friend void from_json(nlohmann::json const& json, Keyframe& keyframe);

        Keyframe() = default;
        Keyframe(Keyframe const&) = default;
        Keyframe(Keyframe&&) = default;
        Keyframe& operator=(Keyframe const&) = default;
        Keyframe& operator=(Keyframe&&) = default;
        ~Keyframe() = default;
    };
}
