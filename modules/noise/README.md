# moth::noise

**Package** `moth_noise` · **Namespace** `moth::noise` · **Umbrella** `<moth/noise/noise.h>` · **CMake target** `moth::noise`

Node-graph noise built on [FastNoise2](https://github.com/Auburn/FastNoise2).
Where `moth/core/noise.h` gives you a dependency-free Perlin or Simplex sample
for when you just want *something*, this module is for applications that depend
on robust, authored noise: full generator graphs — fractals, domain warp,
cellular, blends — loaded from a file and evaluated at runtime. Depends on
`moth_core` + `fastnoise2` (1.1.1) + `nlohmann_json`.

Part of the [Moth Toolkit](../../README.md).

`NodeTree` owns a FastNoise node graph and converts it to and from JSON.
FastNoise's own interchange format is an opaque base64 blob; the JSON form here
is keyed by the node and parameter names FastNoise's metadata reports, so a
saved graph stays readable, diffable and hand-editable. `FromEncodedString` /
`ToEncodedString` bridge to the base64 encoding for interoperating with the
upstream node editor.

The module deliberately depends on `moth_core` only for its types, never its
windowing layer, so an external editor can build against `moth_noise` with
`MOTH_CORE_ENABLE_PLATFORM=OFF` (Conan: `moth_core/*:enable_platform=False`) and
own its own window. The JSON form is the contract between the toolkit and that
tooling.

```cpp
#include <moth/noise/noise.h>

using namespace moth::noise;

// Load an authored graph and evaluate it.
std::ifstream file("terrain.noise.json");
std::string error;
auto tree = NodeTree::FromJson(nlohmann::json::parse(file), &error);
if (!tree) {
    moth::core::log::error("bad noise tree: {}", error);
    return;
}

auto generator = tree->CreateGenerator();
std::vector<float> heights(256 * 256);
generator->GenUniformGrid2D(heights.data(), 0.0f, 0.0f, 256, 256, 0.01f, 0.01f, 1337);

// Or import a tree pasted out of the FastNoise2 node editor.
auto imported = NodeTree::FromEncodedString("EQACAAAA...");
```

## The JSON format

A graph is a DAG, since one node may feed several inputs. The JSON is therefore a
flat node list with integer references rather than nested objects. Enums are
written by name, and per-dimension parameters carry an axis suffix:

```json
{
  "format": "moth.noise.tree",
  "version": 1,
  "root": 0,
  "nodes": [
    {
      "type": "DomainWarpSimplex",
      "variables": {
        "Feature Scale": 600.0,
        "Amplitude Scaling.X": 1.0,
        "Amplitude Scaling.Y": 0.5,
        "Vectorization Scheme": "Orthogonal Gradient Matrix"
      },
      "sources": { "Source": 1 },
      "hybrids": { "Warp Amplitude": { "value": 60.0 } }
    }
  ]
}
```

| Field | Meaning |
|---|---|
| `format` | Always `moth.noise.tree` (`kNodeTreeFormat`) |
| `version` | Schema version (`kNodeTreeVersion`, currently 1) |
| `root` | Index of the output node |
| `nodes[].type` | FastNoise metadata class name, e.g. `FractalFBm` |
| `nodes[].variables` | Plain parameters: numbers, or enum values by name |
| `nodes[].sources` | Node inputs, as indices into `nodes` |
| `nodes[].hybrids` | Parameters that take either a constant (`value`) or a node input |

A member missing from the file keeps the node's default. An unknown node type, a
dangling reference, or an unrecognised `version` is rejected with a reason, never
loaded partially.

`ToJson()` writes only the nodes reachable from the root, numbered in traversal
order, so output is stable across runs and carries no orphaned nodes.

## NodeTree

`NodeTree` is move-only: it owns its nodes, and the graph holds raw pointers
between them.

| Call | Does |
|---|---|
| `FromJson(json, &error)` | Builds a tree from JSON; `std::nullopt` plus a reason on failure |
| `ToJson()` | Serialises to JSON |
| `FromEncodedString(base64)` / `ToEncodedString(fixUp)` | Converts to and from FastNoise's base64 form. `fixUp` asks FastNoise to strip loops and invalid nodes first |
| `CreateGenerator(maxFeatureSet)` | Instantiates a live generator (a null `SmartNode` if the tree is empty or invalid) |
| `IsEmpty()` / `GetRoot()` / `GetNodeCount()` | Inspect the tree |
| `GetNodes()` | The nodes, in the same order as the JSON indices |
| `ReleaseNodes()` | Hands ownership of the nodes out, leaving the tree empty |
| `CopyFrom(root, &sourceOrder)` | Deep-copies a graph you own into a new tree |

## Building an editor on it

An editor keeps its own `FastNoise::NodeData` graph and per-node state such as
canvas positions. Two calls make that round trip work:

- `CopyFrom(root, &sourceOrder)` serialises a graph that is still being edited
  without taking it over. Shared sub-graphs stay shared, and `sourceOrder[i]` is
  the editor's node that became node `i`, so per-node state can be written next
  to the JSON.
- `GetNodes()` after `FromJson` returns the nodes in document order, so per-node
  state can be matched back by index. `ReleaseNodes()` then moves them into the
  editor's own storage.

```cpp
std::vector<FastNoise::NodeData*> order;
nlohmann::json tree = NodeTree::CopyFrom(editorRoot, &order).ToJson();

nlohmann::json positions = nlohmann::json::array();   // saved alongside the tree
for (FastNoise::NodeData* node : order) {
    positions.push_back(PositionOf(node));             // positions[i] belongs to node i
}
```

`FindMetadataByName(name)` looks up a node type by its raw class name.
`MemberKey(member)` returns the JSON key this module uses for a member, including
axis suffixes such as `Scale.X`.

## Using the package

```python
def requirements(self):
    self.requires("moth_noise/0.1.0")
```

```cmake
find_package(moth_noise REQUIRED)
target_link_libraries(my_game PRIVATE moth::noise)
```

## Tests

```bash
cd modules/noise/tests
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```
