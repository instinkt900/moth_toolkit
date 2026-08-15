from conan import ConanFile
from conan.tools.cmake import cmake_layout


class MothPhysicsDemo(ConanFile):
    name = "moth_physics_demo"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("moth_physics/0.1.0")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
