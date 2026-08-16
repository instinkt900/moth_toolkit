#pragma once

#include <algorithm>

namespace moth::core {
    /**
     * @brief A one-shot countdown timer.
     *
     * Counts down from a duration to zero. @c Update(dt) advances the elapsed
     * time; @c IsComplete() becomes true once elapsed reaches the duration and
     * stays true until @c Reset()/@c Start() re-arms it. Good for delays,
     * timeouts, and any "wait N seconds" logic.
     */
    class Timer {
    public:
        Timer() = default;

        /// @brief Constructs a timer armed with @p duration seconds.
        explicit Timer(float duration) : m_duration(duration) {}

        /// @brief Re-arms the timer with a new @p duration and resets elapsed to zero.
        void Start(float duration) {
            m_duration = duration;
            m_elapsed = 0.0f;
        }

        /// @brief Advances the timer by @p dt seconds.
        void Update(float dt) {
            m_elapsed += dt;
        }

        /// @brief Resets elapsed to zero (restarts the same duration).
        void Reset() {
            m_elapsed = 0.0f;
        }

        /// @brief Returns @c true once @p duration has elapsed.
        bool IsComplete() const {
            return m_elapsed >= m_duration;
        }

        /// @brief Returns the elapsed time in seconds.
        float GetElapsed() const {
            return m_elapsed;
        }

        /// @brief Returns the time remaining (>= 0) in seconds.
        float GetRemaining() const {
            return std::max(0.0f, m_duration - m_elapsed);
        }

        /// @brief Returns the total duration in seconds.
        float GetDuration() const {
            return m_duration;
        }

        /// @brief Returns progress in [0, 1] (clamped).
        float Progress() const {
            return m_duration <= 0.0f ? 1.0f : std::min(1.0f, m_elapsed / m_duration);
        }

    private:
        float m_duration = 0.0f;
        float m_elapsed = 0.0f;
    };

    /**
     * @brief Measures elapsed time; can be started, paused, and restarted.
     *
     * @c Start() begins (or resumes) running; @c Stop() pauses; @c Restart()
     * zeroes and begins running; @c Reset() zeroes and stops.
     */
    class Stopwatch {
    public:
        /// @brief Begins (or resumes) accumulating elapsed time.
        void Start() {
            m_running = true;
        }

        /// @brief Pauses accumulation.
        void Stop() {
            m_running = false;
        }

        /// @brief Advances the stopwatch by @p dt if running.
        void Update(float dt) {
            if (m_running) {
                m_elapsed += dt;
            }
        }

        /// @brief Zeroes elapsed and begins running.
        void Restart() {
            m_elapsed = 0.0f;
            m_running = true;
        }

        /// @brief Zeroes elapsed and stops.
        void Reset() {
            m_elapsed = 0.0f;
            m_running = false;
        }

        /// @brief Returns @c true while accumulating.
        bool IsRunning() const {
            return m_running;
        }

        /// @brief Returns the accumulated elapsed time in seconds.
        float GetElapsed() const {
            return m_elapsed;
        }

    private:
        float m_elapsed = 0.0f;
        bool m_running = false;
    };

    /**
     * @brief A reusable cooldown.
     *
     * Starts ready. @c TryFire() succeeds (and restarts the cooldown) only when
     * ready; @c Update(dt) counts the remaining time down toward ready.
     */
    class Cooldown {
    public:
        /// @brief Constructs a cooldown that lasts @p time seconds per use.
        explicit Cooldown(float time = 0.0f) : m_time(time) {}

        /// @brief Advances the cooldown by @p dt seconds.
        void Update(float dt) {
            if (m_remaining > 0.0f) {
                m_remaining = std::max(0.0f, m_remaining - dt);
            }
        }

        /// @brief Returns @c true when the cooldown has fully elapsed.
        bool IsReady() const {
            return m_remaining <= 0.0f;
        }

        /// @brief If ready, restarts the cooldown and returns @c true; otherwise @c false.
        bool TryFire() {
            if (!IsReady()) {
                return false;
            }
            m_remaining = m_time;
            return true;
        }

        /// @brief Makes the cooldown ready immediately (remaining = 0).
        void Reset() {
            m_remaining = 0.0f;
        }

        /// @brief Sets the cooldown duration (applies on the next fire).
        void SetTime(float time) {
            m_time = time;
        }

        /// @brief Returns the cooldown duration.
        float GetTime() const {
            return m_time;
        }

        /// @brief Returns the time remaining until ready (>= 0).
        float GetRemaining() const {
            return m_remaining;
        }

    private:
        float m_time = 0.0f;
        float m_remaining = 0.0f;
    };
}
