from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.system.package_manager import Apt


class MothCoreTests(ConanFile):
    name = "moth_core_tests"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("catch2/3.13.0")
        # moth_core is built from source via add_subdirectory; list its external
        # Conan dependencies here (JSON + fmt for the logging facade). GLFW comes
        # from the system on Linux.
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        self.requires("fmt/[>=10.2 <13]", transitive_headers=True)

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
