// miniaudio is a single-file library; define MINIAUDIO_IMPLEMENTATION in exactly
// one translation unit to pull in the implementation.
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "moth/audio/audio_engine.h"
#include "moth/audio/sound.h"

#include <utility>

namespace moth::audio {
    AudioEngine::AudioEngine(AudioEngineConfig config) {
        m_engine = std::make_unique<ma_engine>();

        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.noDevice = config.noDevice ? MA_TRUE : MA_FALSE;
        if (config.noDevice) {
            // With no device there is nothing to derive channels/sample rate
            // from, so provide them explicitly.
            engineConfig.channels = 2;
            engineConfig.sampleRate = 48000;
        }

        if (ma_engine_init(&engineConfig, m_engine.get()) != MA_SUCCESS) {
            m_engine.reset();
        }
    }

    AudioEngine::~AudioEngine() {
        if (m_engine) {
            ma_engine_uninit(m_engine.get());
        }
    }

    bool AudioEngine::IsValid() const {
        return m_engine != nullptr;
    }

    void AudioEngine::Start() {
        if (m_engine) {
            ma_engine_start(m_engine.get());
        }
    }

    void AudioEngine::Stop() {
        if (m_engine) {
            ma_engine_stop(m_engine.get());
        }
    }

    void AudioEngine::SetMasterVolume(float volume) {
        if (m_engine) {
            ma_engine_set_volume(m_engine.get(), volume);
        }
    }

    float AudioEngine::GetMasterVolume() const {
        return m_engine ? ma_engine_get_volume(m_engine.get()) : 0.0f;
    }

    std::uint32_t AudioEngine::GetSampleRate() const {
        return m_engine ? ma_engine_get_sample_rate(m_engine.get()) : 0u;
    }

    ma_engine& AudioEngine::Raw() {
        return *m_engine;
    }

    Sound AudioEngine::LoadSound(std::filesystem::path const& path) {
        if (!m_engine) {
            return {};
        }

        auto sound = std::make_unique<ma_sound>();
        ma_result const result = ma_sound_init_from_file(
            m_engine.get(), path.string().c_str(),
            MA_SOUND_FLAG_NO_SPATIALIZATION,
            nullptr, nullptr, sound.get());
        if (result != MA_SUCCESS) {
            return {};
        }
        return Sound(std::move(sound), GetSampleRate());
    }

    Sound AudioEngine::LoadMusic(std::filesystem::path const& path) {
        if (!m_engine) {
            return {};
        }

        auto sound = std::make_unique<ma_sound>();
        ma_result const result = ma_sound_init_from_file(
            m_engine.get(), path.string().c_str(),
            MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION,
            nullptr, nullptr, sound.get());
        if (result != MA_SUCCESS) {
            return {};
        }
        return Sound(std::move(sound), GetSampleRate());
    }

    Sound AudioEngine::LoadSoundFromMemory(std::vector<std::uint8_t> const& data) {
        return LoadFromMemory(data, MA_SOUND_FLAG_NO_SPATIALIZATION);
    }

    Sound AudioEngine::LoadMusicFromMemory(std::vector<std::uint8_t> const& data) {
        return LoadFromMemory(data, MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION);
    }

    Sound AudioEngine::LoadFromMemory(std::vector<std::uint8_t> const& data, ma_uint32 flags) {
        if (!m_engine) {
            return {};
        }

        std::vector<std::uint8_t> bytes(data.begin(), data.end());
        auto decoder = std::make_unique<ma_decoder>();
        ma_decoder_config const config = ma_decoder_config_init(ma_format_unknown, 0, 0);
        if (ma_decoder_init_memory(bytes.data(), bytes.size(), &config, decoder.get()) != MA_SUCCESS) {
            return {};
        }

        auto sound = std::make_unique<ma_sound>();
        ma_result const result = ma_sound_init_from_data_source(
            m_engine.get(), decoder.get(), flags, nullptr, sound.get());
        if (result != MA_SUCCESS) {
            return {};
        }

        Sound s(std::move(sound), GetSampleRate());
        s.m_decoder = std::move(decoder);
        s.m_data = std::move(bytes);
        return s;
    }

    Sound::Sound() = default;

    Sound::Sound(std::unique_ptr<ma_sound> sound, std::uint32_t sampleRate)
        : m_sound(std::move(sound))
        , m_sampleRate(sampleRate) {}

    Sound::~Sound() {
        if (m_sound) {
            ma_sound_uninit(m_sound.get());
        }
        if (m_decoder) {
            ma_decoder_uninit(m_decoder.get());
        }
    }

    bool Sound::IsValid() const {
        return m_sound != nullptr;
    }

    void Sound::Play() {
        if (m_sound) {
            ma_sound_start(m_sound.get());
        }
    }

    void Sound::Pause() {
        if (m_sound) {
            ma_sound_stop(m_sound.get());
        }
    }

    void Sound::Stop() {
        if (m_sound) {
            ma_sound_stop(m_sound.get());
            ma_sound_seek_to_pcm_frame(m_sound.get(), 0);
        }
    }

    bool Sound::IsPlaying() const {
        return m_sound && ma_sound_is_playing(m_sound.get());
    }

    void Sound::SetVolume(float volume) {
        if (m_sound) {
            ma_sound_set_volume(m_sound.get(), volume);
        }
    }

    float Sound::GetVolume() const {
        return m_sound ? ma_sound_get_volume(m_sound.get()) : 0.0f;
    }

    void Sound::SetLooping(bool looping) {
        if (m_sound) {
            ma_sound_set_looping(m_sound.get(), looping ? MA_TRUE : MA_FALSE);
        }
    }

    void Sound::SetPitch(float pitch) {
        if (m_sound) {
            ma_sound_set_pitch(m_sound.get(), pitch);
        }
    }

    void Sound::Seek(float seconds) {
        if (m_sound && m_sampleRate > 0) {
            ma_sound_seek_to_pcm_frame(m_sound.get(), static_cast<ma_uint64>(seconds * static_cast<float>(m_sampleRate)));
        }
    }

    float Sound::GetLengthSeconds() const {
        if (!m_sound) {
            return 0.0f;
        }
        ma_uint64 frames = 0;
        if (ma_sound_get_length_in_pcm_frames(m_sound.get(), &frames) != MA_SUCCESS) {
            return 0.0f;
        }
        return m_sampleRate > 0 ? static_cast<float>(frames) / static_cast<float>(m_sampleRate) : 0.0f;
    }
}
