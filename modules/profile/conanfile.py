from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load
import os


class MothProfile(ConanFile):
    name = "moth_profile"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Frame profiling (scope timing and an ImGui profiler panel) for the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    options = {
        "with_imgui": [True, False],
    }
    default_options = {
        "with_imgui": True,
    }

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, os.path.join(self.recipe_folder, "version.txt")).strip()

    def validate(self):
        # Every module is C++17 (CMAKE_CXX_STANDARD 17). Checked here so a profile
        # below it -- MSVC's autodetected default is 14 -- fails with one clear
        # message naming this package, instead of a validation error from each
        # dependency that also needs 17.
        check_min_cppstd(self, 17)

    def requirements(self):
        # The recorder has no dependencies; the profiler panel draws with the
        # ImGui that moth_graphics compiles in.
        if self.options.with_imgui:
            self.requires("moth_graphics/[~2.0]", transitive_headers=True)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["MOTH_PROFILE_ENABLE_IMGUI"] = bool(self.options.with_imgui)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        profile = self.cpp_info.components["profile"]
        profile.set_property("cmake_target_name", "moth::profile")
        profile.libs = ["moth_profile"]
        profile.libdirs = ["lib"]
        profile.includedirs = ["include"]

        if self.options.with_imgui:
            profile_imgui = self.cpp_info.components["profile_imgui"]
            profile_imgui.set_property("cmake_target_name", "moth::profile_imgui")
            profile_imgui.libs = ["moth_profile_imgui"]
            profile_imgui.libdirs = ["lib"]
            profile_imgui.includedirs = ["include"]
            profile_imgui.requires = ["profile", "moth_graphics::moth_graphics"]
