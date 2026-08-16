from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.system.package_manager import Apt


class MothEcsTests(ConanFile):
    name = "moth_ecs_tests"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("catch2/3.13.0")
        # moth_ecs is header-only and built from source via add_subdirectory; its
        # dependencies are moth::core (a package, since our public headers expose
        # core types) and EnTT (header-only).
        self.requires("moth_core/0.1.0", transitive_headers=True)
        self.requires("entt/[~3.13]", transitive_headers=True)

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
