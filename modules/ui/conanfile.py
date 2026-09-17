from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import load
import os

class MothUI(ConanFile):
    name = "moth_ui"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_ui"
    description = "A C++17 UI library with Flash-style keyframe animation, layout serialization, and a graphics-backend-agnostic renderer interface. Designed for games and interactive media applications."

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    package_type = "static-library"

    exports = "README.md", "LICENSE"
    exports_sources = "CMakeLists.txt", "version.txt", "LICENSE", "include/*", "src/*"

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
        # fmt must stay on the 10.2 line: moth::core (which this module links)
        # routes its logging through spdlog 1.14, which pins fmt to 10.2.
        self.requires("moth_core/[~0.1]", transitive_headers=True)
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        self.requires("magic_enum/[~0.8]", transitive_headers=True)
        self.requires("range-v3/[~0.12]", transitive_headers=True)
        self.requires("fmt/[~10.2]", transitive_headers=True)

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
        self.cpp_info.libs = ["moth_ui"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
