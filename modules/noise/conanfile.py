from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load

import os


class MothNoise(ConanFile):
    name = "moth_noise"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Node-graph noise generation (FastNoise2) for the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, os.path.join(self.recipe_folder, "version.txt")).strip()

    def requirements(self):
        # The node graph and its JSON form.
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        # FastNoise2 supplies both the generators and the metadata/reflection
        # system this module serialises against, so its headers are public.
        self.requires("fastnoise2/1.1.1", transitive_headers=True)
        # Only the types are needed here, not a windowing stack — this module is
        # meant to be consumable by external tooling that owns its own window.
        self.requires("moth_core/0.1.0", transitive_headers=True)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_target_name", "moth::noise")
        self.cpp_info.libs = ["moth_noise"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
