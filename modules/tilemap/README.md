# moth::tilemap

**Package** `moth_tilemap` · **Namespace** `moth::tilemap` · **Umbrella** `<moth/tilemap/tilemap.h>` · **CMake target** `moth::tilemap`

Grid-based maps from Tiled `.tmj`: `TileMap`/`Tileset`/`Layer`/`TileId`, a TMJ
importer, culled layered rendering via `IGraphics`, and a world ↔ tile + query
API. Depends on `moth_core` + `moth_graphics` + `nlohmann_json` + `zlib`.

Part of the [Moth Toolkit](../../README.md).

```cpp
#include <moth/tilemap/tilemap.h>

using namespace moth::tilemap;
PropertyTypes types = LoadPropertyTypes("game.tiled-project");  // optional class defaults
TileMap map = LoadTileMapFromFile("maps/level.tmj", types);     // or LoadTileMap(jsonText)

std::vector<moth::gfx::Image> tilesets;           // one Image per tileset, in map order
DrawTileMap(graphics, map, tilesets, viewRect);   // culled, layered draw

for (MapObject const& object : map.objectLayers[0].objects) {
    float speed = GetProperty<float>(object.properties, "speed", 1.0f);
}
```

## Headers

| Header | Contents |
|---|---|
| `tile.h` | `TileId`: a global tile id plus flip flags |
| `tile_map.h` | The data model: `TileMap`, `Tileset`, `Layer`, `Chunk`, `ObjectLayer`, `ImageLayer`, `MapObject` |
| `properties.h` | `Properties`, `PropertyTypes`, `GetProperty`, `HasProperty` |
| `tile_map_loader.h` | `LoadTileMapFromFile`, `LoadTileMap`, `LoadTileMapFromJson`, `LoadPropertyTypes` |
| `tile_map_renderer.h` | `DrawTileMap` |
| `tile_map_collision.h` | `CollectLayerCollisions` |
| `tile_map_physics.h` | Optional Box2D helpers (not in the umbrella header; see [Physics](#physics)) |

The data model and the loader don't touch graphics. Only the renderer uses
`moth::gfx`.

## Loading

The importer supports orthogonal maps with:

- embedded and external (`.tsj`) tilesets, both single-atlas and
  image-collection;
- CSV tile data, or base64 with optional zlib/gzip compression;
- finite maps and infinite (16×16 chunked) maps;
- flip flags, unpacked per tile;
- tile animations, per-tile properties, and per-tile collision shapes;
- layer opacity, tint colour, and parallax factors;
- object layers, including objects placed from templates (`.tj`);
- image layers, with their offset and repeat X/Y flags.

Group layers are skipped. Loading throws `std::runtime_error` (or an
`nlohmann_json` exception) on malformed input.

### Paths

`LoadTileMapFromFile` resolves external tilesets and templates relative to the map
file. `LoadTileMap(jsonText, basePath)` and `LoadTileMapFromJson(json, basePath)`
use `basePath` for the same purpose.

Image paths (`Tileset::imagePath`, `TileImage::imagePath`, `ImageLayer::imagePath`) are returned relative to
the **map file**, including images referenced from a `.tsj` in another directory.
The loader doesn't load images; that's the caller's job.

### Templates

An object placed from a template stores only the template path and the fields and
properties it overrides. The loader merges it over the template's object:
instance fields replace template fields, and properties merge by name. A tile
template's gid is numbered within the template's own tileset reference, so the
loader rebases it onto the map's firstgid for that tileset. If the map doesn't
reference that tileset, loading throws.

### Custom properties

Properties are typed as a `PropertyValue` variant:

| Tiled type | Stored as |
|---|---|
| `bool` | `bool` |
| `int` | `int` |
| `float` | `float` |
| `color` | `moth::core::Color` |
| `object` | `int` (the referenced object's id, 0 for none) |
| `string`, `file`, enums, anything else | `std::string` |
| `class` | skipped (a nested object can't live in a flat map) |

Read them with `GetProperty<T>(props, name, fallback)`. It returns the fallback if
the property is missing **or has a different type**, so an `int` property read as
`float` gives the fallback. Use `HasProperty(props, name)` to test for presence.

### Class defaults

Tiled doesn't write properties left at their class default value. Those defaults
live only in the `.tiled-project`. Load them with `LoadPropertyTypes(projectPath)`
and pass the result to the loader. Defaults then fill in any property that isn't
set on:

- objects (by their class/`type`);
- maps, layers, object layers, image layers, and tilesets (by `class`);
- tiles (by `type`).

Precedence is class default < template < instance. Enums need no definition to
load, and nested class-typed members are skipped.

```cpp
PropertyTypes const types = LoadPropertyTypes("tiled/game.tiled-project");
TileMap const map = LoadTileMapFromFile("tiled/level.tmj", types);
```

## Rendering

`DrawTileMap` draws the tiles visible in `viewRect` (map pixel space). Tile,
object, and image layers are interleaved in their Tiled order. Each layer's opacity and tint
are applied through `SetColor`, and the colour is reset to white afterwards.
Horizontal, vertical, and diagonal flips are honoured. Object layers draw their
tile objects and skip shape objects.

Set the camera transform on `IGraphics` before drawing. There are two overloads:

```cpp
// Atlas tilesets: tilesetImages[i] is the image for map.tilesets[i].
// An empty Image skips that tileset.
DrawTileMap(graphics, map, tilesetImages, viewRect, timeMs, cameraPosition);

// Also handles image-collection tilesets and image layers: the resolver is called
// with each image path (relative to the map). Cache by path; it runs once per draw
// for each atlas tileset and image layer, and once per drawn tile for
// image-collection tilesets. The atlas overload skips image layers.
DrawTileMap(graphics, map, [&](std::string const& path) { return LoadCached(path); },
            viewRect, timeMs, cameraPosition);
```

- `timeMs` is accumulated game time in milliseconds and drives tile animations.
  Leave it 0 for static maps.
- `cameraPosition` is the world-space view centre and drives parallax. Pass
  `Camera::GetPosition()`, or leave it `{}` for maps without parallax.

Finite maps are culled per tile; infinite maps per chunk.

An image layer draws its image at natural size, at the layer offset plus its
parallax offset. With `repeatX` / `repeatY`, the image is tiled along that axis to
cover `viewRect`. Combine repeat with a parallax factor to get an endless scrolling
backdrop or overlay from a small image.

```cpp
graphics.SetTransform(camera.GetViewTransform(viewportSize));
FloatVec2 topLeft, bottomRight;
camera.GetViewportBounds(viewportSize, topLeft, bottomRight);
DrawTileMap(graphics, map, tilesetImages, FloatRect{ topLeft, bottomRight }, timeMs, camera.GetPosition());
```

## Queries

`TileMap` uses map pixels, with +x right and +y down (as in Tiled).

| Call | Returns |
|---|---|
| `WorldToTile(worldPos)` / `TileToWorld(x, y)` | Convert between map pixels and tile coordinates |
| `GetTile(layer, x, y)` / `GetTileAtWorld(layer, worldPos)` | The `TileId` there, or an empty tile if out of bounds |
| `FindTileset(gid)` | The tileset owning a gid, or `nullptr` |
| `GetLayer(i)` / `GetTileset(i)` / `GetObjectLayer(i)` / `GetImageLayer(i)` | Layers, tilesets, object layers, and image layers by index (with `...Count()`) |

`TileId` holds the global id with flags stripped (`id`, 0 = empty) and the
`flipHorizontal` / `flipVertical` / `flipDiagonal` flags. `TileId::FromGid` and
`ToGid` convert to and from Tiled's packed form.

## Physics

`CollectLayerCollisions(map, layerIndex)` returns the collision shapes (from
Tiled's collision editor) of every tile in a layer, offset into world space. They
are plain `MapObject`s, so no physics engine is involved.

`<moth/tilemap/tile_map_physics.h>` turns those shapes into Box2D fixtures
(`AttachCollisionShapes`, `CreateStaticCollisionBody`). The umbrella header doesn't
include it and `moth_tilemap` doesn't depend on Box2D, so only include it in a
project that already requires `box2d`, for example through `moth_physics`.
Coordinates pass through unchanged: scaling to metres and flipping y are up to
you.

## Using the package

```python
def requirements(self):
    self.requires("moth_tilemap/0.1.0")
```

```cmake
find_package(moth_tilemap REQUIRED)
target_link_libraries(my_game PRIVATE moth::tilemap)
```

## Tests

The test project builds `moth_core`, `moth_graphics`, and `moth_tilemap` from
source. It requires `box2d` itself to cover the physics helpers.

```bash
cd modules/tilemap/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

## See also

- [`examples/tilemap_demo/`](../../examples/tilemap_demo/) builds a map in memory,
  loads it through the importer, and renders it under a pannable camera.
