#include "moth/audio/audio.h"

#include <catch2/catch_all.hpp>

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace moth::audio;

namespace {
    constexpr float kPi = 3.14159265f;

    // Writes a short mono 16-bit PCM sine-wave WAV for use in headless tests.
    void WriteSineWav(std::filesystem::path const& path, int sampleRate, float seconds, float frequency) {
        int const numSamples = static_cast<int>(sampleRate * seconds);
        std::vector<std::int16_t> samples(static_cast<std::size_t>(numSamples));
        for (int i = 0; i < numSamples; ++i) {
            samples[static_cast<std::size_t>(i)] = static_cast<std::int16_t>(
                std::sin(2.0f * kPi * frequency * static_cast<float>(i) / static_cast<float>(sampleRate)) * 12000.0f);
        }

        std::ofstream file(path, std::ios::binary);
        auto const write32 = [&](std::uint32_t v) { file.write(reinterpret_cast<char const*>(&v), 4); };
        auto const write16 = [&](std::uint16_t v) { file.write(reinterpret_cast<char const*>(&v), 2); };

        std::uint32_t const dataSize = static_cast<std::uint32_t>(numSamples * 2);
        std::uint32_t const byteRate = static_cast<std::uint32_t>(sampleRate * 2);

        file.write("RIFF", 4);
        write32(36 + dataSize);
        file.write("WAVE", 4);
        file.write("fmt ", 4);
        write32(16);              // fmt chunk size
        write16(1);               // PCM
        write16(1);               // mono
        write32(static_cast<std::uint32_t>(sampleRate));
        write32(byteRate);
        write16(2);               // block align
        write16(16);              // bits per sample
        file.write("data", 4);
        write32(dataSize);
        file.write(reinterpret_cast<char const*>(samples.data()), static_cast<std::streamsize>(dataSize));
    }

    struct TempDir {
        std::filesystem::path path;
        TempDir() : path(std::filesystem::temp_directory_path() / "moth_audio_test") {
            std::filesystem::create_directories(path);
        }
        ~TempDir() { std::filesystem::remove_all(path); }
    };
}

TEST_CASE("AudioEngine: initializes with a null device", "[audio][engine]") {
    AudioEngine engine(AudioEngineConfig{ /* noDevice = */ true });
    REQUIRE(engine.IsValid());
    REQUIRE(engine.GetSampleRate() > 0);
}

TEST_CASE("AudioEngine: master volume round-trips", "[audio][engine]") {
    AudioEngine engine(AudioEngineConfig{ true });
    REQUIRE(engine.GetMasterVolume() == Catch::Approx(1.0f));
    engine.SetMasterVolume(0.5f);
    REQUIRE(engine.GetMasterVolume() == Catch::Approx(0.5f));
}

TEST_CASE("Sound: loads, plays, and pauses", "[audio][sound]") {
    TempDir dir;
    auto const wav = dir.path / "tone.wav";
    WriteSineWav(wav, 44100, 0.5f, 440.0f);

    AudioEngine engine(AudioEngineConfig{ true });
    engine.Start();

    Sound sound = engine.LoadSound(wav);
    REQUIRE(sound.IsValid());
    REQUIRE(sound.GetLengthSeconds() > 0.0f);

    sound.Play();
    REQUIRE(sound.IsPlaying());

    sound.Pause();
    REQUIRE_FALSE(sound.IsPlaying());
}

TEST_CASE("Sound: volume, looping, and pitch are settable", "[audio][sound]") {
    TempDir dir;
    auto const wav = dir.path / "tone.wav";
    WriteSineWav(wav, 44100, 0.5f, 440.0f);

    AudioEngine engine(AudioEngineConfig{ true });
    Sound sound = engine.LoadSound(wav);
    REQUIRE(sound.IsValid());

    sound.SetVolume(0.25f);
    REQUIRE(sound.GetVolume() == Catch::Approx(0.25f));
    sound.SetLooping(true);
    sound.SetPitch(1.5f);
    sound.Seek(0.1f);
}

TEST_CASE("Music: streams from file", "[audio][sound]") {
    TempDir dir;
    auto const wav = dir.path / "music.wav";
    WriteSineWav(wav, 44100, 2.0f, 220.0f);

    AudioEngine engine(AudioEngineConfig{ true });
    Sound music = engine.LoadMusic(wav);
    REQUIRE(music.IsValid());
    REQUIRE(music.GetLengthSeconds() > 0.0f);

    music.Play();
    REQUIRE(music.IsPlaying());
}

TEST_CASE("Sound: loading a missing file yields an invalid sound", "[audio][sound]") {
    AudioEngine engine(AudioEngineConfig{ true });
    Sound sound = engine.LoadSound("/nonexistent/does_not_exist.wav");
    REQUIRE_FALSE(sound.IsValid());
}
