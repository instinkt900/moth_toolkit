from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout


class MothPackedAudioDemo(ConanFile):
    name = "moth_packed_audio_demo"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def validate(self):
        # C++17 is the floor for every moth project.
        check_min_cppstd(self, 17)

    def requirements(self):
        self.requires("moth_audio/[~0.1]")
        self.requires("moth_assets/[~0.1]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
