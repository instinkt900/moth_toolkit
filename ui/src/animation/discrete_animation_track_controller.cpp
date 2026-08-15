#include "common.h"
#include "moth_ui/animation/discrete_animation_track_controller.h"

namespace moth_ui {
    DiscreteAnimationTrackController::DiscreteAnimationTrackController(DiscreteAnimationTrack const& track, std::function<void(std::string_view)> callback)
        : m_track(&track)
        , m_callback(std::move(callback)) {
    }

    void DiscreteAnimationTrackController::SetFrame(float frame) {
        std::string const& value = m_track->GetValueAtFrame(static_cast<int>(frame));
        if (!m_initialized || value != m_lastValue) {
            m_initialized = true;
            m_lastValue = value;
            if (m_callback) {
                m_callback(m_lastValue);
            }
        }
    }

    void DiscreteAnimationTrackController::Reset() {
        m_initialized = false;
        m_lastValue.clear();
    }
}
