from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load


class MothEcs(ConanFile):
    name = "moth_ecs"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "A lightweight EnTT-backed entity-component system for the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "header-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, "version.txt").strip()

    def requirements(self):
        # EnTT is header-only and appears in our public headers, so it must reach
        # consumers transitively. Public headers also expose moth::core types
        # (Transform2D via the Transform component).
        self.requires("entt/[~3.13]", transitive_headers=True)
        self.requires("moth_core/0.1.0", transitive_headers=True)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_target_name", "moth::ecs")
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.includedirs = ["include"]
