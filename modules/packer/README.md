# moth::packer

**Package** `moth_packer` · **Namespace** `moth::packer` · **Header** `<moth/packer/packer.h>` · **CMake target** `moth::packer`

Bin-packs images into texture atlases or flipbook sheets, and extracts individual
sprites back out of a sheet. Works either in memory (`PackToMemory`) or against
the filesystem (`Pack`/`Unpack`, which also write a JSON descriptor). The
`moth_packer` CLI in [`tools/moth_packer`](../../tools/moth_packer) is a front end
over this module. Depends on `moth_core` plus `stb`, `p-ranav-glob`,
`nlohmann_json`, `range-v3` and `spdlog`.

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/packer/packer.h>

std::vector<moth::packer::ImageInput> images = /* name + RGBA pixels */;

moth::packer::PackOptions options;
options.maxWidth = options.maxHeight = 2048;
options.padding = 2;

auto result = moth::packer::PackToMemory(std::move(images), options);
if (result.ok) {
    for (auto const& atlas : result.atlases) {
        // atlas.pixels is width * height * 4 bytes of RGBA
        for (auto const& image : atlas.images) {
            use(image.name, image.rect);   // moth::core::IntRect
        }
    }
}
```

## Public dependency surface

Only the math types reach the public header, so `moth::core` is the module's sole
public dependency — `ImageDetails::dimensions` and `PackedImage::rect` are
`moth::core::IntVec2` and `moth::core::IntRect`. Everything else (stb, glob, the
JSON descriptor, logging) is an implementation detail. A consumer that packs
images links `moth::packer` and `moth::core` and nothing more.

## The moth::ui layout collectors (opt-in)

`CollectImagesFromLayout` and `CollectImagesFromLayoutsDir` walk `moth::ui`
layout files and gather the images they reference. They are the only part of the
module that needs `moth::ui`, so they are **off by default**:

- CMake: `-DMOTH_PACKER_ENABLE_UI=ON` (the superbuild sets this automatically
  when `MOTH_ENABLE_UI` is on)
- Conan: `-o moth_packer/*:with_ui=True`

Either one defines `MOTH_PACKER_HAS_UI`, which gates both the declarations in
`<moth/packer/packer.h>` and their definitions, so guard your calls:

```cpp
#ifdef MOTH_PACKER_HAS_UI
    moth::packer::CollectImagesFromLayout(layoutPath, images);
#endif
```

The other collectors — `CollectImagesFromFile`, `CollectImagesFromGlob` and
`CollectImagesFromDir` — are always available and need nothing beyond
`moth::core`.

## Collecting images

Every collector appends to the list you pass and skips paths already in it, so
several can be combined into one pack. Supported extensions are `.png`, `.jpg`,
`.jpeg`, `.bmp` and `.tga`.

```cpp
std::vector<moth::packer::ImageDetails> images;
moth::packer::CollectImagesFromDir("sprites/", /*recursive=*/true, images);
moth::packer::CollectImagesFromGlob("ui/**/*.png", images);
```

## Packing

`PackType::Atlas` packs into the smallest power-of-two atlases that give the best
area utilisation, spilling into more than one atlas when needed.
`PackType::Flipbook` sorts frames by name, packs them into a single atlas, and
emits a `frames` array plus one auto-generated clip; it fails if the frames do not
all fit, so raise `maxWidth`/`maxHeight` in that case.

Check `PackResult::ok` rather than `atlases.empty()` — with `forceOverwrite` set
and every image oversized, a successful pack legitimately returns no atlases.

## Unpacking

`Unpack` detects sprites in a sheet and writes them out individually. Background
is decided by an explicit `backgroundColor`, by `autoDetectBackground` (sampling
the four corners), or by `alphaThreshold`, in that order of precedence. Connected
non-background pixels are grouped with an 8-connectivity flood fill. Setting
`fixedSpriteWidth`/`fixedSpriteHeight` skips detection and cuts a uniform grid
instead, discarding partial tiles at the right and bottom edges.

## Building and testing

```bash
cd modules/packer/tests
conan install . --build=missing
cmake --preset conan-release
cmake --build build/Release
./build/Release/moth_packer_tests
```

The tests cover the layout collectors, so they build the module with
`MOTH_PACKER_ENABLE_UI=ON`.
