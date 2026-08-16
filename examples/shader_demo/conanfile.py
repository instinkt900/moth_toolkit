from conan import ConanFile
from conan.tools.cmake import cmake_layout


class MothShaderDemo(ConanFile):
    name = "moth_shader_demo"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("moth_graphics/2.0.0")
        self.requires("moth_core/0.1.0")

    def configure(self):
        # The demo compiles a GLSL shader at runtime.
        self.options["moth_graphics"].enable_glslang = True

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
