from conan import ConanFile
from conan.tools.cmake import cmake_layout


class MothUITests(ConanFile):
    name = "moth_ui_tests"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("catch2/3.13.0")
        # moth_ui dependencies (built from source via add_subdirectory)
        self.requires("nlohmann_json/[~3.11]")
        self.requires("magic_enum/[~0.8]")
        self.requires("range-v3/[~0.12]")
        self.requires("fmt/[~10.2]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
