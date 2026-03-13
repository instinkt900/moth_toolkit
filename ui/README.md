# Moth UI

[![Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/build-lib.yml/badge.svg)](https://github.com/instinkt900/moth_ui/actions/workflows/build-lib.yml)
[![Upload Status](https://github.com/instinkt900/moth_ui/actions/workflows/upload-lib.yml/badge.svg)](https://github.com/instinkt900/moth_ui/actions/workflows/upload-lib.yml)

A C++ Flash-like framework for building 2D UIs in graphical applications such as games and media players. It supports keyframed animation, a variety of easing curves, mouse and keyboard events, and custom animation events that let the UI signal back to the application during playback.

![Screenshot 2023-10-14 141636](https://github.com/instinkt900/moth_ui/assets/35185578/a8779a2b-978e-450a-b80a-b0dad4f06306)

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Dependencies](#dependencies)
- [Building the Library](#building-the-library)
  - [Prerequisites](#prerequisites)
  - [Linux](#linux)
  - [Windows](#windows)
- [Running the Tests](#running-the-tests)
- [Integrating into Your Project](#integrating-into-your-project)
- [Implementing the Backend Interfaces](#implementing-the-backend-interfaces)

---

## Overview

Moth UI separates the **layout/animation data model** from the **rendering backend**. You describe your UI in JSON layout files (editable with [moth_editor](https://github.com/instinkt900/moth_editor)), load them at runtime via `NodeFactory`, and drive rendering through a small set of interfaces that you implement for your graphics stack.

Key features:

- Node-based scene graph (`Group`, `NodeRect`, `NodeImage`, `NodeText`, `NodeClip`)
- Flash-style keyframe animation with per-property tracks and 30+ easing curves
- Custom animation events fired during playback for UI/game synchronisation
- Graphics-backend agnostic — bring your own renderer
- Event system with bubbling mouse, keyboard, and custom events

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

See [canyon](https://github.com/instinkt900/canyon) for a reference implementation built on SDL2 and Vulkan.

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

Conan profiles for both platforms are provided in `conan/profiles/`. They configure the compiler, C++ standard, and (on Linux) instruct Conan to install system packages via `apt`.

### Linux

```bash
conan install . --profile conan/profiles/linux_profile --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
cmake --install build --config Release --prefix=<install_path>
```

### Windows

```bash
conan install . --profile conan/profiles/windows_profile --build=missing -s build_type=Release
cmake --preset conan-default
cmake --build --preset conan-release
cmake --install build --config Release --prefix=<install_path>
```

The install step copies headers and the compiled library into `<install_path>`, ready to consume as a standard C++ library.

To publish to a Conan remote instead:

```bash
conan create . --profile conan/profiles/<platform>_profile --build=missing
conan upload moth_ui --remote <remote_name>
```

---

## Running the Tests

The test suite uses [Catch2](https://github.com/catchorg/Catch2) and lives in the `tests/` directory. It is built and run separately from the main library.

```bash
cd tests

# Linux
conan install . --profile ../conan/profiles/linux_profile --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure

# Windows
conan install . --profile ../conan/profiles/windows_profile --build=missing -s build_type=Debug
cmake --preset conan-default
cmake --build --preset conan-debug
ctest --preset conan-debug --output-on-failure
```

CI runs the full test matrix (Linux + Windows) on every pull request via GitHub Actions.

---

## Integrating into Your Project

The recommended approach is via Conan. Add moth_ui as a dependency in your `conanfile.py` or `conanfile.txt`:

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

## Implementing the Backend Interfaces

At minimum you need a `moth_ui::Context` constructed with your implementations of the three factory interfaces:

```cpp
class MyRenderer : public moth_ui::IRenderer { /* ... */ };
class MyImageFactory : public moth_ui::IImageFactory { /* ... */ };
class MyFontFactory : public moth_ui::IFontFactory { /* ... */ };

MyRenderer renderer;
MyImageFactory imageFactory;
MyFontFactory fontFactory;
moth_ui::Context context(renderer, imageFactory, fontFactory);
```

Once you have a context, load and display a layout:

```cpp
auto& factory = moth_ui::NodeFactory::Get();
auto root = factory.Create(context, "path/to/layout.mothui", screenWidth, screenHeight);

// In your render loop:
root->Update(deltaMs);
root->Draw();
```

See the [canyon](https://github.com/instinkt900/canyon) project for a complete reference implementation.
