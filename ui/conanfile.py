from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load

class MothUI(ConanFile):
    name = "moth_ui"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_ui"
    description = "A UI library and toolkit for games."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*"

    def set_version(self):
        self.version = load(self, "version.txt").strip()

    def requirements(self):
        self.requires("nlohmann_json/[~3.11]", transitive_headers=True)
        self.requires("magic_enum/[~0.8]", transitive_headers=True)
        self.requires("range-v3/[~0.12]", transitive_headers=True)
        self.requires("fmt/[~10.2]", transitive_headers=True)

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
        self.cpp_info.libs = ["moth_ui"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
