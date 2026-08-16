from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load


class MothAudio(ConanFile):
    name = "moth_audio"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "A lean miniaudio wrapper for sound and music playback in the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, "version.txt").strip()

    def requirements(self):
        # miniaudio.h appears in our public headers (ma_engine& Raw()), so it must
        # reach consumers transitively.
        self.requires("miniaudio/0.11.18", transitive_headers=True)
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
        self.cpp_info.set_property("cmake_target_name", "moth::audio")
        self.cpp_info.libs = ["moth_audio"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
