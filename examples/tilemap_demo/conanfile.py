from conan import ConanFile
from conan.tools.cmake import cmake_layout


class MothTilemapDemo(ConanFile):
    name = "moth_tilemap_demo"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("moth_tilemap/0.1.0")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
