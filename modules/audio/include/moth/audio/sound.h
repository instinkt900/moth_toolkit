#pragma once

#include <miniaudio.h>

#include <cstdint>
#include <memory>

namespace moth::audio {
    class AudioEngine;

    /**
     * @brief A single playable sound (or music) track, owned by an @c AudioEngine.
     *
     * Create one via @c AudioEngine::LoadSound (decoded) or @c LoadMusic
     * (streamed), then control playback with @c Play/@c Pause/@c Stop. Movable,
     * non-copyable; uninitialized when constructed default.
     */
    class Sound {
    public:
        Sound();
        ~Sound();

        Sound(Sound&&) noexcept = default;
        Sound& operator=(Sound&&) noexcept = default;

        Sound(Sound const&) = delete;
        Sound& operator=(Sound const&) = delete;

        /// @brief Returns @c true if this sound loaded successfully.
        bool IsValid() const;

        /// @brief Starts (or resumes) playback.
        void Play();

        /// @brief Pauses playback, keeping the current position.
        void Pause();

        /// @brief Stops playback and rewinds to the start.
        void Stop();

        /// @brief Returns @c true while playing.
        bool IsPlaying() const;

        /// @brief Sets the volume (0..1).
        void SetVolume(float volume);

        /// @brief Returns the volume (0..1).
        float GetVolume() const;

        /// @brief Enables/disables looping.
        void SetLooping(bool looping);

        /// @brief Sets the playback pitch (1.0 = normal).
        void SetPitch(float pitch);

        /// @brief Seeks to @p seconds from the start.
        void Seek(float seconds);

        /// @brief Returns the track length in seconds (0 if unknown).
        float GetLengthSeconds() const;

    private:
        friend class AudioEngine;
        Sound(std::unique_ptr<ma_sound> sound, std::uint32_t sampleRate);

        std::unique_ptr<ma_sound> m_sound;
        std::uint32_t m_sampleRate = 0;
    };
}
