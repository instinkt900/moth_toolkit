from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.system.package_manager import Apt


class MothTilemapTests(ConanFile):
    name = "moth_tilemap_tests"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("catch2/3.13.0")
        # core, gfx, and tilemap are all built from source via add_subdirectory;
        # list their external Conan dependencies here. moth::core needs
        # nlohmann_json; moth::gfx needs spdlog and the Vulkan stack (GLFW and
        # FreeType/HarfBuzz come from the system package manager on Linux).
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        self.requires("spdlog/[~1.14]", transitive_headers=True)
        if self.settings.os == "Windows":
            self.requires("glfw/3.3.8", transitive_headers=True)
            self.requires("freetype/[~2.13]", transitive_headers=True)
            self.requires("harfbuzz/[~8.3]", transitive_headers=True)
        self.requires("vulkan-headers/1.3.243.0", transitive_headers=True)
        self.requires("vulkan-loader/1.3.243.0")
        self.requires("vulkan-memory-allocator/3.0.1", transitive_headers=True)

    def system_requirements(self):
        if self.settings.os == "Linux":
            packages = [
                "libglfw3-dev",
                "libfreetype-dev",
                "libharfbuzz-dev",
                "pkg-config",
            ]
            apt = Apt(self)
            apt.install(packages)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
