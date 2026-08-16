from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.system.package_manager import Apt


class MothGraphicsTests(ConanFile):
    name = "moth_graphics_tests"
    settings = "os", "compiler", "build_type", "arch"

    options = {"enable_glslang": [True, False]}
    default_options = {"enable_glslang": False}

    def requirements(self):
        self.requires("catch2/3.13.0")
        # moth_graphics is built from source via add_subdirectory;
        # list its external Conan dependencies here.
        self.requires("moth_core/0.1.0")
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
        if self.options.enable_glslang:
            self.requires("glslang/1.3.268.0")
            # glslang's public SPIR-V headers include spirv-tools/libspirv.h,
            # so spirv-tools' headers must be visible while we build.
            self.requires("spirv-tools/1.3.268.0")

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

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.cache_variables["MOTH_GRAPHICS_ENABLE_GLSLANG"] = bool(self.options.enable_glslang)
        tc.generate()

    def layout(self):
        cmake_layout(self)
