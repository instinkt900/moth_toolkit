#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <utility>

namespace moth::core {
    namespace detail {
        // A minimal deterministic PRNG for shuffling the permutation table, so the
        // noise classes stay self-contained and don't depend on moth::core::Random.
        struct SplitMix64 {
            std::uint64_t state;

            explicit SplitMix64(std::uint64_t seed) : state(seed) {}

            std::uint64_t Next() {
                std::uint64_t z = (state += 0x9E3779B97F4A7C15ull);
                z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
                z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
                return z ^ (z >> 31);
            }
        };

        /// @brief Perlin's smoothstep (quintic fade curve).
        inline float Fade(float t) {
            return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
        }

        inline float Lerp(float a, float b, float t) {
            return a + t * (b - a);
        }

        /// @brief One of the 8 gradients of Ken Perlin's improved 2D noise.
        inline float PerlinGrad(int hash, float x, float y) {
            switch (hash & 7) {
                case 0: return x + y;
                case 1: return -x + y;
                case 2: return x - y;
                case 3: return -x - y;
                case 4: return x;
                case 5: return -x;
                case 6: return y;
                default: return -y;
            }
        }

        /// @brief One of the 8 gradients of 2D simplex noise.
        inline float SimplexGrad(int hash, float x, float y) {
            int const h = hash & 7;
            float const u = h < 4 ? x : y;
            float const v = h < 4 ? y : x;
            return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
        }

        /// @brief Floors @p x toward negative infinity (works for negative values).
        inline int FastFloor(float x) {
            int const i = static_cast<int>(x);
            return x < i ? i - 1 : i;
        }
    }

    /**
     * @brief Classic 2D Perlin gradient noise (Ken Perlin's improved noise).
     *
     * Output is smooth, deterministic, and zero at integer lattice points, with
     * values roughly in [-1, 1]. Reseeding derives a fresh permutation from the
     * seed, so different seeds give different but reproducible noise.
     */
    class PerlinNoise {
    public:
        /// @brief Constructs noise with @p seed.
        explicit PerlinNoise(std::uint32_t seed = 0) {
            Reseed(seed);
        }

        /// @brief Regenerates the permutation from @p seed.
        void Reseed(std::uint32_t seed) {
            std::array<int, 256> perm{};
            for (int i = 0; i < 256; ++i) {
                perm[i] = i;
            }
            detail::SplitMix64 rng(seed);
            for (int i = 255; i > 0; --i) {
                std::swap(perm[i], perm[static_cast<int>(rng.Next() % static_cast<std::uint64_t>(i + 1))]);
            }
            for (int i = 0; i < 512; ++i) {
                m_perm[i] = perm[i & 255];
            }
        }

        /// @brief Samples the noise at (@p x, @p y), roughly in [-1, 1].
        float Noise(float x, float y) const {
            int const xi = detail::FastFloor(x) & 255;
            int const yi = detail::FastFloor(y) & 255;
            float const xf = x - std::floor(x);
            float const yf = y - std::floor(y);
            float const u = detail::Fade(xf);
            float const v = detail::Fade(yf);

            int const aa = m_perm[m_perm[xi] + yi];
            int const ab = m_perm[m_perm[xi] + yi + 1];
            int const ba = m_perm[m_perm[xi + 1] + yi];
            int const bb = m_perm[m_perm[xi + 1] + yi + 1];

            float const x1 = detail::Lerp(detail::PerlinGrad(aa, xf, yf), detail::PerlinGrad(ba, xf - 1.0f, yf), u);
            float const x2 = detail::Lerp(detail::PerlinGrad(ab, xf, yf - 1.0f), detail::PerlinGrad(bb, xf - 1.0f, yf - 1.0f), u);
            return detail::Lerp(x1, x2, v);
        }

        /// @brief Fractal (multi-octave) noise, normalized to roughly [-1, 1].
        float Fractal(float x, float y, int octaves = 4, float lacunarity = 2.0f, float gain = 0.5f) const {
            float amplitude = 1.0f;
            float frequency = 1.0f;
            float sum = 0.0f;
            float norm = 0.0f;
            for (int i = 0; i < octaves; ++i) {
                sum += amplitude * Noise(x * frequency, y * frequency);
                norm += amplitude;
                amplitude *= gain;
                frequency *= lacunarity;
            }
            return sum / norm;
        }

    private:
        std::array<int, 512> m_perm{};
    };

    /**
     * @brief 2D simplex noise.
     *
     * Lower distortion than classic Perlin noise, with values in [-1, 1].
     * Deterministic and seedable like @c PerlinNoise.
     */
    class SimplexNoise {
    public:
        /// @brief Constructs noise with @p seed.
        explicit SimplexNoise(std::uint32_t seed = 0) {
            Reseed(seed);
        }

        /// @brief Regenerates the permutation from @p seed.
        void Reseed(std::uint32_t seed) {
            std::array<int, 256> perm{};
            for (int i = 0; i < 256; ++i) {
                perm[i] = i;
            }
            detail::SplitMix64 rng(seed);
            for (int i = 255; i > 0; --i) {
                std::swap(perm[i], perm[static_cast<int>(rng.Next() % static_cast<std::uint64_t>(i + 1))]);
            }
            for (int i = 0; i < 512; ++i) {
                m_perm[i] = perm[i & 255];
            }
        }

        /// @brief Samples 2D simplex noise at (@p x, @p y), in [-1, 1].
        float Noise(float x, float y) const {
            constexpr float F2 = 0.36602540378f; // (sqrt(3) - 1) / 2
            constexpr float G2 = 0.21132486540f; // (3 - sqrt(3)) / 6

            float const s = (x + y) * F2;
            int const i = detail::FastFloor(x + s);
            int const j = detail::FastFloor(y + s);
            float const t = static_cast<float>(i + j) * G2;
            float const x0 = x - (static_cast<float>(i) - t);
            float const y0 = y - (static_cast<float>(j) - t);

            int i1, j1;
            if (x0 > y0) {
                i1 = 1;
                j1 = 0;
            } else {
                i1 = 0;
                j1 = 1;
            }

            float const x1 = x0 - static_cast<float>(i1) + G2;
            float const y1 = y0 - static_cast<float>(j1) + G2;
            float const x2 = x0 - 1.0f + 2.0f * G2;
            float const y2 = y0 - 1.0f + 2.0f * G2;

            int const ii = i & 255;
            int const jj = j & 255;

            float n0 = 0.0f;
            float n1 = 0.0f;
            float n2 = 0.0f;

            float t0 = 0.5f - x0 * x0 - y0 * y0;
            if (t0 > 0.0f) {
                t0 *= t0;
                n0 = t0 * t0 * detail::SimplexGrad(m_perm[ii + m_perm[jj]], x0, y0);
            }

            float t1 = 0.5f - x1 * x1 - y1 * y1;
            if (t1 > 0.0f) {
                t1 *= t1;
                n1 = t1 * t1 * detail::SimplexGrad(m_perm[ii + i1 + m_perm[jj + j1]], x1, y1);
            }

            float t2 = 0.5f - x2 * x2 - y2 * y2;
            if (t2 > 0.0f) {
                t2 *= t2;
                n2 = t2 * t2 * detail::SimplexGrad(m_perm[ii + 1 + m_perm[jj + 1]], x2, y2);
            }

            // The 3-vertex sum is bounded by ~0.0223 regardless of permutation, so
            // this scale keeps the output within [-1, 1] (the classic 70.0 factor
            // only bounds the reference permutation).
            return 44.0f * (n0 + n1 + n2);
        }

        /// @brief Fractal (multi-octave) noise, normalized to roughly [-1, 1].
        float Fractal(float x, float y, int octaves = 4, float lacunarity = 2.0f, float gain = 0.5f) const {
            float amplitude = 1.0f;
            float frequency = 1.0f;
            float sum = 0.0f;
            float norm = 0.0f;
            for (int i = 0; i < octaves; ++i) {
                sum += amplitude * Noise(x * frequency, y * frequency);
                norm += amplitude;
                amplitude *= gain;
                frequency *= lacunarity;
            }
            return sum / norm;
        }

    private:
        std::array<int, 512> m_perm{};
    };
}
