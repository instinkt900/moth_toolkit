// Audio demo: generates a couple of sine-wave WAVs, then plays one as a one-shot
// sound and the other as looping streaming music. Exercises moth::audio (the
// miniaudio wrapper) end to end from a single main.cpp.

#include <moth/audio/audio.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

using namespace moth::audio;

namespace {
    constexpr float kPi = 3.14159265f;

    // Writes a mono 16-bit PCM sine-wave WAV.
    void WriteToneWav(std::filesystem::path const& path, int sampleRate, float seconds, float frequency) {
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
        file.write("RIFF", 4);
        write32(36 + dataSize);
        file.write("WAVE", 4);
        file.write("fmt ", 4);
        write32(16);
        write16(1);
        write16(1);
        write32(static_cast<std::uint32_t>(sampleRate));
        write32(static_cast<std::uint32_t>(sampleRate * 2));
        write16(2);
        write16(16);
        file.write("data", 4);
        write32(dataSize);
        file.write(reinterpret_cast<char const*>(samples.data()), static_cast<std::streamsize>(dataSize));
    }

    // A short 4-note loop (C-E-G-C) so "music" is audibly distinct from the blip.
    void WriteMelodyWav(std::filesystem::path const& path, int sampleRate) {
        constexpr float noteFreqs[4] = { 261.63f, 329.63f, 392.00f, 523.25f };
        constexpr float noteSeconds = 0.4f;

        int const notesPerNote = static_cast<int>(sampleRate * noteSeconds);
        int const numSamples = notesPerNote * 4;
        std::vector<std::int16_t> samples(static_cast<std::size_t>(numSamples));

        for (int n = 0; n < 4; ++n) {
            for (int i = 0; i < notesPerNote; ++i) {
                samples[static_cast<std::size_t>(n * notesPerNote + i)] = static_cast<std::int16_t>(
                    std::sin(2.0f * kPi * noteFreqs[n] * static_cast<float>(i) / static_cast<float>(sampleRate)) * 9000.0f);
            }
        }

        std::ofstream file(path, std::ios::binary);
        auto const write32 = [&](std::uint32_t v) { file.write(reinterpret_cast<char const*>(&v), 4); };
        auto const write16 = [&](std::uint16_t v) { file.write(reinterpret_cast<char const*>(&v), 2); };

        std::uint32_t const dataSize = static_cast<std::uint32_t>(numSamples * 2);
        file.write("RIFF", 4);
        write32(36 + dataSize);
        file.write("WAVE", 4);
        file.write("fmt ", 4);
        write32(16);
        write16(1);
        write16(1);
        write32(static_cast<std::uint32_t>(sampleRate));
        write32(static_cast<std::uint32_t>(sampleRate * 2));
        write16(2);
        write16(16);
        file.write("data", 4);
        write32(dataSize);
        file.write(reinterpret_cast<char const*>(samples.data()), static_cast<std::streamsize>(dataSize));
    }
}

int main(int argc, char** argv) {
    bool const nullDevice = (argc > 1 && std::strcmp(argv[1], "--null") == 0);

    // Generate two WAVs in a temp directory.
    auto const dir = std::filesystem::temp_directory_path() / "moth_audio_demo";
    std::filesystem::create_directories(dir);
    auto const blipPath = dir / "blip.wav";
    auto const musicPath = dir / "music.wav";
    WriteToneWav(blipPath, 44100, 0.2f, 880.0f);
    WriteMelodyWav(musicPath, 44100);

    AudioEngine engine(AudioEngineConfig{ nullDevice });
    if (!engine.IsValid()) {
        std::printf("failed to initialize the audio engine\n");
        std::filesystem::remove_all(dir);
        return 1;
    }

    std::printf("audio demo: playing a one-shot blip and looping music (%s device)\n",
                nullDevice ? "null" : "real");
    engine.Start();

    Sound blip = engine.LoadSound(blipPath);
    Sound music = engine.LoadMusic(musicPath);
    if (!blip.IsValid() || !music.IsValid()) {
        std::printf("failed to load the demo tracks\n");
        std::filesystem::remove_all(dir);
        return 1;
    }

    music.SetLooping(true);
    music.SetVolume(0.4f);
    music.Play();
    blip.Play();

    // Re-trigger the blip every second for ~5 seconds.
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        blip.Stop();
        blip.Play();
    }

    music.Stop();
    blip.Stop();
    engine.Stop();

    std::filesystem::remove_all(dir);
    std::printf("audio demo finished\n");
    return 0;
}
