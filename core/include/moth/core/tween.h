#pragma once

#include "moth/core/color.h"
#include "moth/core/interp.h"
#include "moth/core/vector.h"

namespace moth::core {
    /**
     * @brief A time-based value tween: interpolates from @c From to @c To over a
     * duration using an @c InterpType easing curve.
     *
     * @c Update(dt) advances the tween; @c GetValue() returns the eased value at
     * the current progress (clamped so it ends exactly on @c To). Reuses the
     * existing @c interp.h easing curves via @c Interp.
     *
     * @tparam T Any type with the arithmetic of @c Vector (e.g. @c float,
     *           @c FloatVec2, @c Color).
     */
    template <typename T>
    class Tween {
    public:
        Tween() = default;

        /// @brief Constructs a tween from @p from to @p to over @p duration seconds.
        Tween(T from, T to, float duration, InterpType ease = InterpType::Linear)
            : m_from(from)
            , m_to(to)
            , m_duration(duration)
            , m_ease(ease) {}

        /// @brief (Re)starts the tween with new endpoints, duration, and easing.
        void Start(T from, T to, float duration, InterpType ease = InterpType::Linear) {
            m_from = from;
            m_to = to;
            m_duration = duration;
            m_ease = ease;
            m_elapsed = 0.0f;
        }

        /// @brief Advances the tween by @p dt seconds.
        void Update(float dt) {
            m_elapsed += dt;
            if (m_elapsed > m_duration) {
                m_elapsed = m_duration;
            }
        }

        /// @brief Returns the eased value at the current progress.
        T GetValue() const {
            return Interp(m_from, m_to, Progress(), m_ease);
        }

        /// @brief Returns @c true when the tween has reached the end.
        bool IsComplete() const {
            return m_elapsed >= m_duration;
        }

        /// @brief Returns progress in [0, 1].
        float Progress() const {
            return m_duration <= 0.0f ? 1.0f : m_elapsed / m_duration;
        }

        /// @brief Replays the tween from the beginning.
        void Restart() {
            m_elapsed = 0.0f;
        }

        /// @brief Returns the start value.
        T From() const {
            return m_from;
        }

        /// @brief Returns the end value.
        T To() const {
            return m_to;
        }

        /// @brief Returns the total duration in seconds.
        float GetDuration() const {
            return m_duration;
        }

        /// @brief Returns the elapsed time in seconds.
        float GetElapsed() const {
            return m_elapsed;
        }

    private:
        T m_from{};
        T m_to{};
        float m_duration = 0.0f;
        float m_elapsed = 0.0f;
        InterpType m_ease = InterpType::Linear;
    };

    /// @brief Convenience tween over a single @c float.
    using FloatTween = Tween<float>;

    /// @brief Convenience tween over a 2D vector (@c FloatVec2).
    using Vec2Tween = Tween<FloatVec2>;

    /// @brief Convenience tween over an RGBA colour (@c Color).
    using ColorTween = Tween<Color>;
}
