from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import load
import os


class MothTilemap(ConanFile):
    name = "moth_tilemap"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Grid-based tilemaps and tilesets (Tiled .tmj import + rendering) for the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    package_type = "static-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, os.path.join(self.recipe_folder, "version.txt")).strip()

    def validate(self):
        # Every module is C++17 (cxx_std_17). Checked here so a profile
        # below it -- MSVC's autodetected default is 14 -- fails with one clear
        # message naming this package, instead of a validation error from each
        # dependency that also needs 17.
        check_min_cppstd(self, 17)

    def requirements(self):
        # TMJ parsing uses nlohmann_json; the renderer uses moth::gfx types. Both
        # appear in our public headers, so they must reach consumers. zlib is a
        # compiled link dependency (compressed tile data) and stays private.
        # box2d is deliberately not required: the optional tile_map_physics.h
        # header is for consumers that already depend on it (e.g. moth_physics).
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        self.requires("zlib/1.3.2")
        self.requires("moth_core/[~0.1]", transitive_headers=True)
        self.requires("moth_graphics/[~2.0]", transitive_headers=True)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_target_name", "moth::tilemap")
        self.cpp_info.libs = ["moth_tilemap"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
