# Moth UI
![Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/build-lib.yml/badge.svg) 
![Upload Status](https://github.com/instinkt900/moth_ui/actions/workflows/upload-lib.yml/badge.svg)

A C++ Flash-like framework for building 2D UIs for graphical applications such as games and media. It supports basic keyframed animations and various mouse and keyboard events. Custom events can also be fired off back to a listening application during animations to handle synchronization between the UI and application.

![Screenshot 2023-10-14 141636](https://github.com/instinkt900/moth_ui/assets/35185578/a8779a2b-978e-450a-b80a-b0dad4f06306)

### Building the Moth UI library

#### Conan

The easiest way to build is by using Conan and CMake. You can refer to either Conan docs or even the github workflows on this project to get the exact commands, but basically the flow for building release on Windows is as follows...

##### Setup Conan

```Linux
python3 -m venv .venv
source ./.venv/bin/activate
pip3 install conan
```
```Windows
python3 -m venv .venv
.\venv\Scripts\activate.bat
pip3 install conan
```

##### Build library

```Linux
conan install . --build=missing -s build_type=Release [--profile <profile>]
cmake --preset conan-release
cmake --build --preset conan-release
cmake --install build --config Release --prefix=<install_path>
```
```Windows
conan install . --build=missing -s build_type=Release [--profile <profile>]
cmake --preset conan-default
cmake --build --preset conan-release
cmake --install build --config Release --prefix=<install_path>
```
This should build the library and copy the headers into `install_path` which can be consumed like any other C++ library.
Conan profiles are provided in the `conan` directory and provide the basic conan environment for the build.

