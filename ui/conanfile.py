from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain
from conan.tools.files import load

class MothUI(ConanFile):
    name = "moth_ui"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*"
    package_type = "static-library"

    def set_version(self):
        self.version = load(self, "version.txt").strip()

    def requirements(self):
        self.requires("nlohmann_json/3.11.2", transitive_headers=True)
        self.requires("magic_enum/0.7.3", transitive_headers=True)
        self.requires("range-v3/0.12.0", transitive_headers=True)
        self.requires("fmt/10.0.0", transitive_headers=True)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)

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
