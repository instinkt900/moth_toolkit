from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load


class MothAssets(ConanFile):
    name = "moth_assets"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Asset addressing (id/path) and loading for the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, "version.txt").strip()

    def requirements(self):
        # The manifest is written as JSON (nlohmann_json is already a core dep).
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
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
        self.cpp_info.set_property("cmake_target_name", "moth::assets")
        self.cpp_info.libs = ["moth_assets"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
