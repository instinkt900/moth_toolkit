from conan import ConanFile
from conan.tools.cmake import cmake_layout


class MothPackedAudioDemo(ConanFile):
    name = "moth_packed_audio_demo"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("moth_audio/[~0.1]")
        self.requires("moth_assets/[~0.1]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
