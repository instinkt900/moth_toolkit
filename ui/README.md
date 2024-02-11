# Moth UI
![Windows Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/cmake-build-lib-win.yml/badge.svg) 
![Linux Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/cmake-build-lib-linux.yml/badge.svg) 
![Windows Editor Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/cmake-build-editor-win.yml/badge.svg) 
![Linux Editor Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/cmake-build-editor-linux.yml/badge.svg)  
A C++ Flash-like framework for building 2D UIs for graphical applications such as games and media. It supports basic keyframed animations and various mouse and keyboard events. Custom events can also be fired off back to a listening application during animations to handle synchronization between the UI and application.

![Screenshot 2023-10-14 141636](https://github.com/instinkt900/moth_ui/assets/35185578/a8779a2b-978e-450a-b80a-b0dad4f06306)

### Building the Moth UI library

#### Conan

The easiest way to build is by using Conan and CMake. You can refer to either Conan docs or even the github workflows on this project to get the exact commands, but basically the flow for building release on Windows is as follows...
```
conan install . --build=missing -s build_type=Release [--profile <profile>]
cmake --preset conan-default
cmake --build --preset conan-release
cmake --install build --config Release --prefix=<install_path>
```
The exact commands vary with Linux and Windows but it should be easy to figure out what needs to change.
This should build the library and copy the headers into `install_path` which can be consumed like any other C++ library.

#### Visual Studio

The Visual Studio solution is provided in the `projects/vs` path. It includes moth_ui, the backends, and the editor. Check the editor building section for the full setup.

## Editor

### Building the Moth UI Editor

#### Conan

Building the editor is the same workflow as the library itself, only from the editor folder.
```
cd editor
conan install . --build=missing -s build_type=Release [--profile <profile>]
cmake --preset conan-default
cmake --build --preset conan-release
cmake --install build --config Release --prefix=<install_path>
```
Like with the library build this should install the editor in the given `install_path` folder.

#### Visual Studio

If using Visual Studio you will need to install the following dependencies...
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- [SDL2](https://github.com/libsdl-org/SDL)
- [SDL Image](https://github.com/libsdl-org/SDL_image)
- [SDL TTF](https://github.com/libsdl-org/SDL_ttf)

Once you have installed the dependencies there is a convenience script `project/vs/prereqs.bat` that you can run which will build all the `external` dependencies that need to be built and update the `moth_ui.props` file to the expected install paths of the given dependencies.
Once the script has completed you must update the following macros to match your installation paths for the above dependencies...
- VULKAN_DIR
- SDL_DIR
- SDLIMAGE_DIR
- SDLTTF_DIR

When these all point to the correct versions of the libraries you should just be able to open the `moth_ui.sln` file and build.

You might wish to install the `external` dependency libraries to your own paths. The following is a list of libraries that currently require building...
- fmt
- freetype
- harfbuzz-icu-freetype
- spdlog

You can use the following command in each path to install to your own location
```
cmake .. && cmake --build . --config Release && cmake --install . --config Release --prefix=install_path
```
If you do install these to custom locations you will need to update the `moth_ui.props` file to locate them.

### Using the editor
TODO
