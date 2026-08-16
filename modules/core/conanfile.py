from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load

import shutil


class MothCore(ConanFile):
    name = "moth_core"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Core math, platform, windowing, and event types for the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, "version.txt").strip()

    def requirements(self):
        # JSON serialisation of the core math types (Vector/Rect).
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        # GLFW windowing backend: system on Linux, Conan on Windows.
        if self.settings.os == "Windows":
            self.requires("glfw/3.3.8", transitive_headers=True)

    def system_requirements(self):
        if self.settings.os == "Linux":
            if not shutil.which("pkg-config"):
                raise ConanInvalidConfiguration(
                    "pkg-config is required to locate GLFW on Linux. "
                    "Install it with: sudo apt install pkg-config")

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
        # Expose the target as moth::core (matching the CMake install/export and
        # the superbuild ALIAS) so consumers link one consistent name.
        self.cpp_info.set_property("cmake_target_name", "moth::core")
        self.cpp_info.libs = ["moth_core"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
        if self.settings.os == "Linux":
            self.cpp_info.system_libs = ["glfw"]
