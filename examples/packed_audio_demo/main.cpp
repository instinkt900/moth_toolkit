// Packed audio demo: writes a WAV, cooks it into a .pak + manifest, deletes the
// original file, then loads and plays it back by id — no asset path on disk.
//
// Exercises the Phase 8 asset pipeline end to end:
//   folder -> PackDirectory -> .pak -> PackedAssetSource::Load -> bytes -> LoadSoundFromMemory -> play

#include <moth/audio/audio.h>
#include <moth/assets/pak.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

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
        write32(16);              // fmt chunk size
        write16(1);               // PCM
        write16(1);               // mono
        write32(static_cast<std::uint32_t>(sampleRate));
        write32(static_cast<std::uint32_t>(sampleRate * 2));
        write16(2);               // block align
        write16(16);              // bits per sample
        file.write("data", 4);
        write32(dataSize);
        file.write(reinterpret_cast<char const*>(samples.data()), static_cast<std::streamsize>(dataSize));
    }
} // namespace

int main(int argc, char** argv) {
    bool const nullDevice = (argc > 1 && std::strcmp(argv[1], "--null") == 0);

    // 1. Write a source asset in a staging folder.
    auto const root = std::filesystem::temp_directory_path() / "moth_packed_audio_demo";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "assets");
    auto const wav = root / "assets" / "tone.wav";
    WriteToneWav(wav, 44100, 0.3f, 440.0f);

    // 2. Cook the folder into a .pak + manifest, then delete the original file.
    auto const pak = root / "assets.pak";
    auto const manifest = root / "manifest.json";
    moth::assets::PackDirectory(root / "assets", pak, manifest);
    std::filesystem::remove_all(root / "assets");

    // 3. Load the archive and resolve the asset by id (no path on disk).
    auto source = moth::assets::PackedAssetSource::Load(pak);
    if (!source.IsValid() || source.GetEntryCount() == 0) {
        std::printf("failed to load the .pak\n");
        std::filesystem::remove_all(root);
        return 1;
    }

    moth::assets::AssetId const id = moth::assets::MakeAssetId("tone.wav");
    auto bytes = source.Read(id);
    if (bytes.empty()) {
        std::printf("failed to read the asset by id\n");
        std::filesystem::remove_all(root);
        return 1;
    }

    // Sanity: path addressing resolves to the same bytes.
    if (source.Read("tone.wav") != bytes) {
        std::printf("path- and id-addressed reads differ\n");
        std::filesystem::remove_all(root);
        return 1;
    }

    // 4. Play it back from memory.
    moth::audio::AudioEngine engine(moth::audio::AudioEngineConfig{ nullDevice });
    if (!engine.IsValid()) {
        std::printf("failed to initialize the audio engine\n");
        std::filesystem::remove_all(root);
        return 1;
    }
    engine.Start();

    moth::audio::Sound sound = engine.LoadSoundFromMemory(bytes);
    if (!sound.IsValid()) {
        std::printf("failed to load the packed sound\n");
        std::filesystem::remove_all(root);
        return 1;
    }

    std::printf("packed audio demo: playing 'tone.wav' by id 0x%016llx from %s (%s device)\n",
                static_cast<unsigned long long>(id.value), pak.string().c_str(),
                nullDevice ? "null" : "real");
    sound.Play();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    sound.Stop();
    engine.Stop();

    std::filesystem::remove_all(root);
    std::printf("packed audio demo finished\n");
    return 0;
}
