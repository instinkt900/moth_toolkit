#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/utils/interp.h"

#include <nlohmann/json_fwd.hpp>

namespace moth_ui {
    /**
     * @brief A single keyframe on an animation track.
     *
     * A keyframe stores the frame index, the scalar value at that frame, and
     * the easing curve used to interpolate toward the next keyframe.
     */
    struct Keyframe {
        /**
         * @brief Constructs a keyframe at the given frame with the given value.
         * @param frame Frame index (zero-based).
         * @param value Scalar value at this frame.
         */
        Keyframe(int frame, KeyframeValue value)
            : m_frame(frame)
            , m_value(value) {}

        int m_frame = 0;                              ///< Frame index at which this keyframe is set.
        KeyframeValue m_value = 0;                    ///< Scalar value at this keyframe.
        InterpType m_interpType = InterpType::Linear; ///< Easing curve used to reach the next keyframe.

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
