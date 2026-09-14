from conan import ConanFile
from conan.tools.cmake import cmake_layout


class MothProfileTests(ConanFile):
    name = "moth_profile_tests"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        # moth_profile is built from source via add_subdirectory with the ImGui
        # panel turned off, so the recorder needs nothing else.
        self.requires("catch2/3.13.0")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
