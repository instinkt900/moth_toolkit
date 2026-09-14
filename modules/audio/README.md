# moth::audio

**Package** `moth_audio` · **Namespace** `moth::audio` · **Umbrella** `<moth/audio/audio.h>` · **CMake target** `moth::audio`

A miniaudio wrapper: `AudioEngine` (device/node graph, master volume, null-device
mode) and `Sound` (decoded one-shot or streamed music; play/pause/stop/volume/
looping/pitch/seek/length). Depends on `moth_core` + `miniaudio` (0.11.18).

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/audio/audio.h>

using namespace moth::audio;
AudioEngine engine;                       // AudioEngineConfig{ true } (noDevice) for headless
engine.Start();

Sound blip  = engine.LoadSound("blip.wav");
Sound music = engine.LoadMusic("theme.ogg");
music.SetVolume(0.4f);
music.Play();
blip.Play();
```

## AudioEngine

One engine owns the audio device and miniaudio's node graph. Create it, start it,
and load sounds through it. The engine is movable but not copyable.

| Call | Does |
|---|---|
| `AudioEngine(config = {})` | Initialises the engine. Check `IsValid()` |
| `Start()` / `Stop()` | Starts or stops the audio device |
| `SetMasterVolume(v)` / `GetMasterVolume()` | Master volume, 0..1 |
| `GetSampleRate()` | The engine's sample rate in Hz |
| `LoadSound(path)` | Loads a fully decoded sound (short effects) |
| `LoadMusic(path)` | Loads a streamed sound (long tracks) |
| `LoadSoundFromMemory(bytes)` / `LoadMusicFromMemory(bytes)` | The same, from encoded bytes in memory |
| `Raw()` | The underlying `ma_engine` |

Loading never starts playback. Decoding is done by miniaudio, which has WAV,
FLAC, and MP3 built in. Other formats, such as Ogg Vorbis, need a decoder that
miniaudio was built with.

### Headless mode

Set `AudioEngineConfig::noDevice` to use a null device with no audio output. That
lets tests and tools run the full load and playback path on machines without a
sound card.

```cpp
AudioEngineConfig config;
config.noDevice = true;
AudioEngine engine(config);
```

## Sound

A `Sound` is one playable track created by an engine. It's movable but not
copyable, and a default-constructed `Sound` is invalid. Destroy sounds before the
engine that created them.

| Call | Does |
|---|---|
| `IsValid()` | `true` if the sound loaded |
| `Play()` / `Pause()` / `Stop()` | Start or resume / pause in place / stop and rewind |
| `IsPlaying()` | `true` while playing |
| `SetVolume(v)` / `GetVolume()` | Volume, 0..1 |
| `SetLooping(bool)` | Loop at the end |
| `SetPitch(p)` | Playback pitch (1.0 = normal) |
| `Seek(seconds)` | Jump to a position |
| `GetLengthSeconds()` | Track length, or 0 if unknown |

## Loading from packed assets

The `...FromMemory` loaders take encoded bytes, so they work with any byte source.
The sound copies the bytes and keeps them alive for as long as it exists. With
`moth::assets`:

```cpp
auto pak = moth::assets::PackedAssetSource::Load("assets.pak");
Sound jump = engine.LoadSoundFromMemory(pak.Read("sfx/jump.wav"));
```

## Using the package

```python
def requirements(self):
    self.requires("moth_audio/0.1.0")
```

```cmake
find_package(moth_audio REQUIRED)
target_link_libraries(my_game PRIVATE moth::audio)
```

miniaudio's header is part of the public API (for `Raw()`), so it reaches
consumers automatically.

## Tests

```bash
cd modules/audio/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

## See also

- [`examples/audio_demo/`](../../examples/audio_demo/): sound effects and music
  from files.
- [`examples/packed_audio_demo/`](../../examples/packed_audio_demo/): cook a
  `.pak`, then load and play by id.
