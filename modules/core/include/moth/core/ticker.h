#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

namespace moth::core {
    /// @brief Fixed-timestep update loop.
    ///
    /// Drives two callbacks at different rates:
    /// - @c TickFixed() is called at a guaranteed fixed interval (default 60 Hz),
    ///   potentially multiple times per frame to catch up on missed ticks.
    /// - @c Tick() is called once per iteration with the remaining fractional
    ///   milliseconds, suitable for rendering.
    ///
    /// Call @c TickSync() to start the loop. It blocks until @c SetRunning(false)
    /// is called (typically from within an event handler).
    class Ticker {
    public:
        /// @param ticksPerSecond Fixed update rate in Hz. Clamped to 60 if <= 0.
        /// The period is tracked in microseconds so common rates (60/144/240 Hz)
        /// are represented accurately rather than truncated to whole milliseconds.
        explicit Ticker(int ticksPerSecond = 60)
            : m_updateTicks(std::chrono::microseconds(1000000 / (ticksPerSecond > 0 ? ticksPerSecond : 60))) {
        }
        virtual ~Ticker() {}

        /// @brief Returns the fixed tick interval in milliseconds (rounded).
        uint32_t GetFixedTicks() const { return static_cast<uint32_t>((m_updateTicks.count() + 500) / 1000); }

        /// @brief Start or stop the loop. Call @c SetRunning(false) to exit @c TickSync().
        void SetRunning(bool running) { m_running = running; }

        /// @brief Run the loop synchronously until @c SetRunning(false) is called.
        void TickSync() {
            m_running = true;
            m_lastUpdateTicks = std::chrono::steady_clock::now();
            while (m_running) {
                auto const nowTicks = std::chrono::steady_clock::now();
                auto deltaTicks = std::chrono::duration_cast<std::chrono::microseconds>(nowTicks - m_lastUpdateTicks);
                int catchUpCount = 0;
                while (deltaTicks >= m_updateTicks && catchUpCount < kMaxCatchUpTicks) {
                    TickFixed(GetFixedTicks());
                    m_lastUpdateTicks += m_updateTicks;
                    deltaTicks -= m_updateTicks;
                    ++catchUpCount;
                }
                if (deltaTicks >= m_updateTicks) {
                    // Stalled for a long time (e.g. a debugger pause): drop the
                    // backlog rather than bursting hundreds of fixed ticks, then
                    // resynchronise to now.
                    m_lastUpdateTicks = nowTicks;
                    deltaTicks = std::chrono::microseconds::zero();
                }
                Tick(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(deltaTicks).count()));
                auto const sleepFor = m_updateTicks - deltaTicks;
                if (sleepFor > std::chrono::microseconds::zero()) {
                    std::this_thread::sleep_for(sleepFor);
                }
            }
        }

    protected:
        /// @brief Called at the fixed update rate.
        /// @param ticks Fixed tick interval in milliseconds (equal to @c GetFixedTicks()).
        virtual void TickFixed(uint32_t ticks) = 0;

        /// @brief Called once per loop iteration with the fractional elapsed time.
        /// @param ticks Milliseconds elapsed since the last fixed tick catchup.
        virtual void Tick(uint32_t ticks) = 0;

    private:
        static constexpr int kMaxCatchUpTicks = 100;

        std::atomic<bool> m_running = false;
        std::chrono::microseconds m_updateTicks;
        std::chrono::time_point<std::chrono::steady_clock> m_lastUpdateTicks;
    };
}
