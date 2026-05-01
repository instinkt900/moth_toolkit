# Moth UI

[![Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/build-test.yml/badge.svg)](https://github.com/instinkt900/moth_ui/actions/workflows/build-test.yml)
[![Upload Status](https://github.com/instinkt900/moth_ui/actions/workflows/upload-release.yml/badge.svg)](https://github.com/instinkt900/moth_ui/actions/workflows/upload-release.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A C++17 library for building 2D UIs in graphical applications such as games and media players. It supports Flash-style keyframe animation, a variety of easing curves, mouse and keyboard events, and custom animation events that let the UI signal back to the application during playback.

---

## Table of Contents

- [Overview](#overview)
  - [AI Disclosure](#ai-disclosure)
- [Architecture](#architecture)
- [Thread Safety](#thread-safety)
- [Dependencies](#dependencies)
- [Using with Conan](#using-with-conan)
- [Building the Library](#building-the-library)
  - [Prerequisites](#prerequisites)
  - [Linux](#linux)
  - [Windows](#windows)
- [Running the Tests](#running-the-tests)
- [Building the Documentation](#building-the-documentation)
- [Implementing the Backend Interfaces](#implementing-the-backend-interfaces)
- [Including in Your Code](#including-in-your-code)
- [Related Projects](#related-projects)
- [License](#license)

---

## Overview

Moth UI separates the **layout/animation data model** from the **rendering backend**. You describe your UI in JSON layout files (editable with [moth_editor](https://github.com/instinkt900/moth_editor)), load them at runtime via `NodeFactory`, and drive rendering through a small set of interfaces that you implement for your graphics stack.

Key features:

- Node-based scene graph (`Group`, `NodeRect`, `NodeImage`, `NodeText`, `NodeClip`, `NodeFlipbook`)
- Flash-style keyframe animation with per-property tracks and 30+ easing curves
- Custom animation events fired during playback for UI/game synchronisation
- Graphics-backend agnostic — bring your own renderer
- Event system with bubbling mouse, keyboard, and custom events

### AI Disclosure

AI agents (primarily Claude) are used as tools in this project for tasks such as refactoring, documentation writing, and test implementation. The architecture, design decisions, and direction of the project are human-driven. This is not a vibe-coded project.

---

## Architecture

The library is split into three logical layers:

| Layer | Responsibility |
|---|---|
| **Layout entities** (`LayoutEntity` subclasses) | Serialisable data model — loaded from JSON, holds keyframe tracks |
| **Nodes** (`Node` subclasses) | Live scene graph — instantiated from layout entities, driven by `AnimationController` |
| **Context** | Ties together the asset factories (`IImageFactory`, `IFontFactory`, `IRenderer`) passed to every node |

Consumers must provide concrete implementations of:

- `moth_ui::IRenderer` — all drawing commands
- `moth_ui::IImage` / `moth_ui::IImageFactory` — image asset loading
- `moth_ui::IFont` / `moth_ui::IFontFactory` — font asset loading

See [moth_graphics](https://github.com/instinkt900/moth_graphics) for a reference implementation built on SDL2 and Vulkan.

---

## Thread Safety

The library is designed around a typical game/application pattern: layout factories are configured on startup, and the UI is driven from a single render thread.

| Component | Thread safe? |
|---|---|
| `NodeFactory::Get()` | Yes — singleton initialisation uses a function-local static (C++11 magic static) |
| `NodeFactory::RegisterWidget()` | Yes — guarded by an internal `std::shared_mutex` |
| `NodeFactory::Create()` | Yes — concurrent calls are safe; creation callbacks are invoked outside the lock |
| `SetLogger()` / `GetLogger()` | Yes — backed by `std::atomic<ILogger*>` |
| `LayoutCache` | Yes — all methods are guarded by an internal `std::mutex` |
| `Node` trees (update, draw, event dispatch) | **No** — all operations on a node tree must happen on a single thread |

The recommended pattern is:
- Register custom widget types from the main thread before spawning any worker threads.
- Drive all node update/draw/event calls from a single render thread.

---

## Dependencies

| Dependency | Version | Source |
|---|---|---|
| nlohmann_json | ≥ 3.11 | Conan |
| fmt | ≥ 10.2 | Conan |
| magic_enum | ≥ 0.8 | Conan |
| range-v3 | ≥ 0.12 | Conan |

**Toolchain requirements:** CMake 3.27+, Conan 2.x, C++17 compiler.

---

## Using with Conan

The recommended way to consume moth_ui is as a Conan package. Add it as a dependency in your `conanfile.py`:

```python
def requirements(self):
    self.requires("moth_ui/<version>")
```

Or in a `conanfile.txt`:

```ini
[requires]
moth_ui/<version>
```

Then link against the `moth_ui` target in CMake:

```cmake
find_package(moth_ui REQUIRED)
target_link_libraries(my_app PRIVATE moth_ui::moth_ui)
```

---

## Building the Library

### Prerequisites

Set up a Python virtual environment and install Conan:

```bash
# Linux / macOS
python3 -m venv .venv
source .venv/bin/activate
pip install conan

# Windows (PowerShell)
python3 -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install conan
```

**C++17 is required.** A `.conan/profile` is provided that sets `compiler.cppstd=17`. This profile is used in CI and can be used directly or as a reference when building locally.

### Linux

```bash
conan install . -pr .conan/profile -s build_type=Release --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
cmake --install build --config Release --prefix=<install_path>
```

### Windows

```bash
conan install . -pr .conan/profile -s build_type=Release --build=missing
cmake --preset conan-default
cmake --build --preset conan-release
cmake --install build --config Release --prefix=<install_path>
```

The install step copies headers and the compiled library into `<install_path>`, ready to consume as a standard C++ library.

---

## Running the Tests

The test suite uses [Catch2](https://github.com/catchorg/Catch2) and lives in the `tests/` directory.

```bash
cd tests

# Linux
conan install . -pr .conan/profile -s build_type=Debug --build=missing
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure

# Windows
conan install . -pr .conan/profile -s build_type=Debug --build=missing
cmake --preset conan-default
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

CI runs the full test matrix (Linux + Windows) on every pull request via GitHub Actions.

---

## Building the Documentation

If [Doxygen](https://www.doxygen.nl) is installed, a `docs` target is available after configuring with CMake:

```bash
cmake --build --preset conan-release --target docs
```

The generated HTML is written to `build/docs/html/index.html`.

---

## Implementing the Backend Interfaces

At minimum you need a `moth_ui::Context` constructed with your implementations of the three factory interfaces:

```cpp
class MyRenderer : public moth_ui::IRenderer { /* ... */ };
class MyImageFactory : public moth_ui::IImageFactory { /* ... */ };
class MyFontFactory : public moth_ui::IFontFactory { /* ... */ };

MyRenderer renderer;
MyImageFactory imageFactory;
MyFontFactory fontFactory;
moth_ui::Context context(&imageFactory, &fontFactory, &renderer);
```

Once you have a context, load and display a layout:

```cpp
auto& factory = moth_ui::NodeFactory::Get();
auto root = factory.Create(context, "path/to/layout.mothui", screenWidth, screenHeight);

// In your render loop:
root->Update(deltaMs);
root->Draw();
```

See the [moth_graphics](https://github.com/instinkt900/moth_graphics) project for a complete reference implementation.

---

## Including in Your Code

Two convenience headers are provided so you can choose the right level of detail:

**All public types and implementations — use in `.cpp` files or when you need the full API:**
```cpp
#include "moth_ui/moth_ui.h"
```

**Forward declarations only — use in headers to minimise compile-time dependencies:**
```cpp
#include "moth_ui/moth_ui_fwd.h"
```

You can also include individual headers directly from `moth_ui/animation/`, `moth_ui/nodes/`, `moth_ui/layout/`, etc.

---

## Related Projects

| Project | Description |
|---|---|
| moth_ui | *(this project)* Core UI library — node graph, keyframe animation, and event system |
| [moth_graphics](https://github.com/instinkt900/moth_graphics) | Graphics and application framework built on moth_ui — SDL2 and Vulkan backends, window management, and a layer stack |
| [moth_editor](https://github.com/instinkt900/moth_editor) | Visual layout and animation editor — Flash-like authoring tool for creating moth_ui layout files |
| [moth_packer](https://github.com/instinkt900/moth_packer) | Command-line texture atlas packer for images and moth_ui layouts |

---

## License

This project is licensed under the [MIT License](LICENSE).
