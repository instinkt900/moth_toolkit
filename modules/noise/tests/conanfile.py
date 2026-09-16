from conan import ConanFile
from conan.tools.cmake import cmake_layout


class MothNoiseTests(ConanFile):
    name = "moth_noise_tests"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("catch2/3.13.0")
        # moth_noise is built from source via add_subdirectory; its deps are
        # moth::core, nlohmann_json and fastnoise2. The platform backend is off
        # so the tests do not need a windowing stack.
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        self.requires("fastnoise2/1.1.1", transitive_headers=True)
        self.requires("moth_core/[~0.1]", transitive_headers=True)

    def configure(self):
        self.options["moth_core"].enable_platform = False

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
