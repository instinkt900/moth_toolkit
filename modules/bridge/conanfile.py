from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load
import os


class MothBridge(ConanFile):
    name = "moth_bridge"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Adapter wiring moth_ui and the graphics window together."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, os.path.join(self.recipe_folder, "version.txt")).strip()

    def validate(self):
        # Every module is C++17 (CMAKE_CXX_STANDARD 17). Checked here so a profile
        # below it -- MSVC's autodetected default is 14 -- fails with one clear
        # message naming this package, instead of a validation error from each
        # dependency that also needs 17.
        check_min_cppstd(self, 17)

    def requirements(self):
        # Public headers expose moth::core/moth::gfx/moth::ui types, so their
        # headers must reach our consumers.
        self.requires("moth_core/[~0.1]", transitive_headers=True)
        self.requires("moth_graphics/[~2.0]", transitive_headers=True)
        self.requires("moth_ui/[~2.0]", transitive_headers=True)

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
        self.cpp_info.set_property("cmake_target_name", "moth::bridge")
        self.cpp_info.libs = ["moth_bridge"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
