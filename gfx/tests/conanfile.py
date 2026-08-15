from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.system.package_manager import Apt


class MothGraphicsTests(ConanFile):
    name = "moth_graphics_tests"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("catch2/3.13.0")
        # moth_graphics is built from source via add_subdirectory;
        # list its external Conan dependencies here. Order matters: spdlog
        # pins fmt/10.2.x, so declare it before moth_ui (whose wider fmt range
        # otherwise resolves to fmt/12 and conflicts).
        self.requires("spdlog/[~1.14]", transitive_headers=True)
        self.requires("moth_core/0.1.0")
        self.requires("moth_ui/1.1.2", transitive_headers=True)
        # GLFW/Freetype/HarfBuzz come from the system package manager on Linux
        # (GTK3/GDK-Pixbuf conflict). On Windows they come from Conan.
        if self.settings.os == "Windows":
            self.requires("glfw/3.3.8", transitive_headers=True)
            self.requires("freetype/[~2.13]", transitive_headers=True)
            self.requires("harfbuzz/[~8.3]", transitive_headers=True)
        # Vulkan packages have no GTK3 conflict — use Conan on both platforms.
        self.requires("vulkan-headers/1.3.243.0", transitive_headers=True)
        self.requires("vulkan-loader/1.3.243.0")
        self.requires("vulkan-memory-allocator/3.0.1", transitive_headers=True)

    def system_requirements(self):
        if self.settings.os == "Linux":
            packages = [
                "libglfw3-dev", "libfreetype-dev", "libharfbuzz-dev",
                "pkg-config",
            ]
            apt = Apt(self)
            apt.install(packages)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
