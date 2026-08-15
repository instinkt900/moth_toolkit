from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load


class MothPhysics(ConanFile):
    name = "moth_physics"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "A Box2D-backed 2D physics module for the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, "version.txt").strip()

    def requirements(self):
        # Box2D headers appear in our public headers, so they must reach consumers.
        self.requires("box2d/2.4.1", transitive_headers=True)
        # Public headers expose moth::core types (FloatVec2).
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
        self.cpp_info.set_property("cmake_target_name", "moth::physics")
        self.cpp_info.libs = ["moth_physics"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
