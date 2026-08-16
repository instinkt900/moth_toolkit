#pragma once

#include <cstdint>
#include <random>

namespace moth::core {
    /**
     * @brief A deterministic, seedable pseudo-random number generator.
     *
     * Wraps @c std::mt19937_64, which is a fixed algorithm — the same seed always
     * produces the same sequence. The floating-point helpers derive directly from
     * the raw engine output rather than @c std::uniform_real_distribution (whose
     * exact values are not specified), so @c NextFloat() is reproducible too.
     */
    class Random {
    public:
        using result_type = std::uint64_t;

        /// @brief Constructs a generator with @p seed.
        explicit Random(std::uint64_t seed = 0xC0FFEE) : m_engine(seed) {}

        /// @brief Reseeds the generator, restarting the sequence.
        void Seed(std::uint64_t seed) {
            m_engine.seed(seed);
        }

        /// @brief Returns the next raw value in [0, 2^64).
        std::uint64_t NextUInt() {
            return m_engine();
        }

        /// @brief Returns a uniform integer in [0, @p upperBoundExclusive).
        ///
        /// @p upperBoundExclusive must be greater than zero. Uses a modulo, so the
        /// result is slightly biased when the bound does not divide 2^64 — fine
        /// for game use; use @c NextInt for an unbiased bounded range.
        std::uint64_t NextUInt(std::uint64_t upperBoundExclusive) {
            return m_engine() % upperBoundExclusive;
        }

        /// @brief Returns a uniform integer in [@p min, @p max] (inclusive).
        int NextInt(int min, int max) {
            return std::uniform_int_distribution<int>(min, max)(m_engine);
        }

        /// @brief Returns a uniform float in [0, 1).
        float NextFloat() {
            // The top 24 bits give a value in [0, 2^24), mapped exactly to [0, 1).
            return static_cast<float>(m_engine() >> 40) * (1.0f / 16777216.0f);
        }

        /// @brief Returns a uniform float in [@p min, @p max).
        float NextFloat(float min, float max) {
            return min + NextFloat() * (max - min);
        }

        /// @brief Returns @c true roughly half the time.
        bool NextBool() {
            return (m_engine() & 1u) != 0;
        }

        /// @brief Returns the underlying engine for use with the standard distributions.
        std::mt19937_64& Engine() {
            return m_engine;
        }

    private:
        std::mt19937_64 m_engine;
    };
}
