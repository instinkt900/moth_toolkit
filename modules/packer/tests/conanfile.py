from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout


class MothPackerTests(ConanFile):
    name = "moth_packer_tests"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def validate(self):
        # C++17 is the floor for every moth project.
        check_min_cppstd(self, 17)

    def requirements(self):
        self.requires("catch2/3.13.0")
        # moth_packer's own dependencies; the library itself is pulled in via
        # add_subdirectory rather than as a package.
        self.requires("moth_core/[~0.1]")
        self.requires("moth_ui/[~2.0]")
        self.requires("stb/cci.20240531")
        self.requires("p-ranav-glob/0.0.1")
        self.requires("nlohmann_json/[>=3.11 <4]")
        self.requires("range-v3/[~0.12]")
        self.requires("spdlog/[~1.14]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
