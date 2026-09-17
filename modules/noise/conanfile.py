from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import load

import os


class MothNoise(ConanFile):
    name = "moth_noise"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Node-graph noise generation (FastNoise2) for the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    package_type = "static-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, os.path.join(self.recipe_folder, "version.txt")).strip()

    def configure(self):
        # FastNoise2's CMake installs its PDB directory inside an unconditional
        # if(MSVC) block with no option to turn it off. A static MSVC build never
        # creates that directory, so fastnoise2's package() fails:
        #
        #   file INSTALL cannot find ".../build/pdb-files/Release"
        #
        # Built shared, the linker writes the PDB the install expects. Nothing on
        # ConanCenter avoids it: 1.1.1 has a single recipe revision. Set here
        # rather than in CI so that anyone building moth_noise on Windows gets it.
        if self.settings.os == "Windows":
            self.options["fastnoise2/*"].shared = True

    def validate(self):
        # Every module is C++17 (cxx_std_17). Checked here so a profile
        # below it -- MSVC's autodetected default is 14 -- fails with one clear
        # message naming this package, instead of a validation error from each
        # dependency that also needs 17.
        check_min_cppstd(self, 17)

    def requirements(self):
        # The node graph and its JSON form.
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        # FastNoise2 supplies both the generators and the metadata/reflection
        # system this module serialises against, so its headers are public.
        self.requires("fastnoise2/1.1.1", transitive_headers=True)
        # Only the types are needed here, not a windowing stack — this module is
        # meant to be consumable by external tooling that owns its own window.
        self.requires("moth_core/[~0.1]", transitive_headers=True)

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
        self.cpp_info.set_property("cmake_target_name", "moth::noise")
        self.cpp_info.libs = ["moth_noise"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
