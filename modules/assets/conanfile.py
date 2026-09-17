from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import load
import os


class MothAssets(ConanFile):
    name = "moth_assets"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Asset addressing (id/path) and loading for the Moth toolkit."

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
        # The manifest is written as JSON (nlohmann_json is already a core dep).
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
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
        self.cpp_info.set_property("cmake_target_name", "moth::assets")
        self.cpp_info.libs = ["moth_assets"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
