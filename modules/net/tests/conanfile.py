from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout
from conan.tools.system.package_manager import Apt


class MothNetTests(ConanFile):
    name = "moth_net_tests"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def validate(self):
        # C++17 is the floor for every moth project.
        check_min_cppstd(self, 17)

    def requirements(self):
        self.requires("catch2/3.13.0")
        # moth_net is built from source via add_subdirectory; its deps are
        # moth::core, nlohmann_json, and asio.
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        self.requires("asio/1.30.2", transitive_headers=True)
        self.requires("moth_core/[~0.1]", transitive_headers=True)

    def system_requirements(self):
        if self.settings.os == "Linux":
            packages = [
                "libglfw3-dev",
                "pkg-config",
            ]
            apt = Apt(self)
            apt.install(packages)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
