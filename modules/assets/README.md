# moth::assets

**Package** `moth_assets` · **Namespace** `moth::assets` · **Umbrella** `<moth/assets/assets.h>` (+ `<moth/assets/pak.h>`) · **CMake target** `moth::assets`

Id/path asset addressing and a `.pak` pack format. Each asset's id is the FNV-1a
hash of its relative path, so it can be addressed by id or path. Cook a folder
with `moth_pak`, then load the archive at runtime via `PackedAssetSource` and
feed the bytes to any `...FromMemory` loader. Depends on `moth_core` +
`nlohmann_json`.

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/assets/assets.h>
#include <moth/assets/pak.h>

auto source = moth::assets::PackedAssetSource::Load("data.pak");
auto bytes = source.Read(idOrPath);       // by AssetId or path string
engine.LoadSoundFromMemory(bytes);        // any FromMemory loader
```

## Design

The module deals only in bytes. A source hands back the raw contents of an asset
and knows nothing about what they mean; decoding is the job of the loader you feed
them to (`AssetContext::TextureFromMemory`, `AudioEngine::LoadSoundFromMemory`,
and so on). A project can start by loading loose files by path and switch to a
packed archive later without changing how assets are named.

## Asset ids

`AssetId` wraps a stable 64-bit value. `MakeAssetId(name)` derives it with 64-bit
FNV-1a. It's `constexpr`, so ids can be computed at compile time, and it gives the
same result on every platform and in every session.

```cpp
constexpr moth::assets::AssetId kJumpSound = moth::assets::MakeAssetId("sfx/jump.wav");
```

Ids hash the path **exactly as written**, relative to the packed root, so
`"sfx/jump.wav"` and `"./sfx/jump.wav"` are different ids. The packer uses paths
relative to the input folder with forward slashes on every platform, so use the
same form at runtime.

## Sources

`AssetSource` is the interface: `Read(path)` / `Exists(path)`, plus optional
`Read(id)` / `Exists(id)` (by default these return empty / `false`). `Read`
returns empty bytes if the asset doesn't exist.

| Source | Addressing | Use |
|---|---|---|
| `PhysicalAssetSource(root = ".")` | path only | Loose files under a directory |
| `PackedAssetSource` | id and path (the path is hashed to the id) | A cooked `.pak` archive |

`PackedAssetSource::Load(pakPath)` and `FromBytes(bytes)` return an invalid source
on failure; check `IsValid()`. `GetEntryCount()` returns the number of assets.

### AssetLibrary

`AssetLibrary` searches several mounted sources in order, and the **first** source
that has an asset wins. Later sources are fallbacks.

```cpp
moth::assets::AssetLibrary library;
library.Mount(std::make_unique<moth::assets::PackedAssetSource>(
    moth::assets::PackedAssetSource::Load("assets.pak")));
library.MountDirectory("assets");     // falls back to loose files

auto bytes = library.Read("textures/player.png");
```

Reads by id only consult id-addressed sources.

## Packing

### moth_pak

The `moth_pak` CLI cooks a directory. Build it in the superbuild with
`-DMOTH_ENABLE_TOOLS=ON`.

```bash
moth_pak <input-dir> [--pak PATH] [--manifest PATH]
```

It writes `assets.pak` and `manifest.json` to the current directory by default.

### From code

`pak.h` exposes the same pipeline:

| Call | Does |
|---|---|
| `PackDirectory(root, pakPath, manifestPath)` | Packs every regular file under `root` |
| `WritePak(entries)` | Serialises `PakEntry`s (id, path, type, bytes) to `.pak` bytes |
| `WriteManifest(entries, manifestPath)` | Writes the manifest |

### Format

A `.pak` file has three parts:

1. A 16-byte header: the magic `MOTHPAK1`, a `uint32` entry count, and 4 reserved
   bytes.
2. An index sorted by id, with one entry per asset: a `uint64` id, a `uint64`
   offset, and a `uint64` size.
3. The asset bytes, back to back.

`manifest.json` (format `version` 1) maps each id, in hex, to its source path and
an extension-derived type. The runtime doesn't need it; it's there so tools and
people can see what an id refers to.

## Using the package

```python
def requirements(self):
    self.requires("moth_assets/0.1.0")
```

```cmake
find_package(moth_assets REQUIRED)
target_link_libraries(my_game PRIVATE moth::assets)
```

## Tests

```bash
cd modules/assets/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

## See also

- [`examples/packed_audio_demo/`](../../examples/packed_audio_demo/): a complete
  cook → load-by-id → play loop.
- [`tools/moth_pak/`](../../tools/moth_pak/)
