# Moth UI
![Windows Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/cmake-build-lib-win.yml/badge.svg) 
![Linux Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/cmake-build-lib-linux.yml/badge.svg) 
![Windows Editor Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/cmake-build-editor-win.yml/badge.svg) 
![Linux Editor Build Status](https://github.com/instinkt900/moth_ui/actions/workflows/cmake-build-editor-linux.yml/badge.svg)  
A C++ Flash-like framework for building 2D UIs for graphical applications such as games and media. It supports basic keyframed animations and various mouse and keyboard events. Custom events can also be fired off back to a listening application during animations to handle synchronization between the UI and application.

### Building
The easiest way to build is by using Conan and CMake.
```
mkdir build && cd build
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
cmake --build . --config Release
cmake --install . --config Release --prefix=install_path
```
This should build the library and copy the headers into `install_path` which can be consumed like any other C++ library.

### Editor
An editor is also supplied in the `editor` folder. This allows you to build the layouts to use with the library. Proper documentation will be provided at a later date.

Building the editor is the same as building the library except in the `editor` folder.
