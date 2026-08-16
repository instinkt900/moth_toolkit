#pragma once

#include <miniaudio.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace moth::audio {
    class Sound;

    /// @brief Options for creating an @c AudioEngine.
    struct AudioEngineConfig {
        bool noDevice = false; ///< When true, use a null device (no audio output).
    };

    /**
     * @brief Owns a miniaudio engine: the audio device plus its node graph.
     *
     * Create one engine, start it, and play @c Sound / music tracks through it.
     * The engine is non-copyable but movable. Use @c AudioEngineConfig::noDevice
     * for a null (dummy) device — useful for headless tests and tools with no
     * audio output.
     */
    class AudioEngine {
    public:
        /// @brief Initializes the engine with @p config.
        explicit AudioEngine(AudioEngineConfig config = {});

        ~AudioEngine();

        AudioEngine(AudioEngine&&) noexcept = default;
        AudioEngine& operator=(AudioEngine&&) noexcept = default;

        AudioEngine(AudioEngine const&) = delete;
        AudioEngine& operator=(AudioEngine const&) = delete;

        /// @brief Returns @c true if the engine initialized successfully.
        bool IsValid() const;

        /// @brief Starts the audio device (playback begins).
        void Start();

        /// @brief Stops the audio device.
        void Stop();

        /// @brief Sets the master volume (0..1).
        void SetMasterVolume(float volume);

        /// @brief Returns the master volume (0..1).
        float GetMasterVolume() const;

        /// @brief Returns the engine's sample rate in Hz.
        std::uint32_t GetSampleRate() const;

        /// @brief Returns the underlying miniaudio engine.
        ma_engine& Raw();

        /// @brief Loads (without playing) a fully-decoded sound from @p path.
        Sound LoadSound(std::filesystem::path const& path);

        /// @brief Loads (without playing) a streaming sound (music) from @p path.
        Sound LoadMusic(std::filesystem::path const& path);

        /// @brief Loads (without playing) a fully-decoded sound from in-memory bytes.
        Sound LoadSoundFromMemory(std::vector<std::uint8_t> const& data);

        /// @brief Loads (without playing) a streaming sound (music) from in-memory bytes.
        Sound LoadMusicFromMemory(std::vector<std::uint8_t> const& data);

    private:
        Sound LoadFromMemory(std::vector<std::uint8_t> const& data, ma_uint32 flags);

        std::unique_ptr<ma_engine> m_engine;
    };
}
