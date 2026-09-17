from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import load
import os


class MothPacker(ConanFile):
    name = "moth_packer"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Texture atlas/flipbook packing and sprite-sheet extraction for the Moth toolkit."

    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    # The moth::ui layout collectors are opt-in; see MOTH_PACKER_ENABLE_UI in
    # CMakeLists.txt. Off by default so image-only consumers do not pull in the
    # UI module and its dependency tree.
    options = {"with_ui": [True, False]}
    default_options = {"with_ui": False}

    exports_sources = "CMakeLists.txt", "version.txt", "include/*", "src/*", "cmake/*"

    def set_version(self):
        if not self.version:
            self.version = load(self, os.path.join(self.recipe_folder, "version.txt")).strip()

    def validate(self):
        # Every module is C++17 (cxx_std_17). Checked here so a profile
        # below it -- MSVC's autodetected default is 14 -- fails with one clear
        # message naming this package, instead of a validation error from each
        # dependency that also needs 17.
        check_min_cppstd(self, 17)

    def requirements(self):
        # Only the math types reach the public header, so moth_core is the sole
        # dependency a consumer inherits.
        self.requires("moth_core/[~0.1]", transitive_headers=True)
        # Implementation details: image load/write and bin packing (stb), glob
        # expansion, the JSON descriptor, and logging.
        self.requires("stb/cci.20240531")
        self.requires("p-ranav-glob/0.0.1")
        self.requires("nlohmann_json/[>=3.11 <4]")
        self.requires("range-v3/[~0.12]")
        self.requires("spdlog/[~1.14]")
        if self.options.with_ui:
            self.requires("moth_ui/[~2.0]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.cache_variables["MOTH_PACKER_ENABLE_UI"] = bool(self.options.with_ui)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_target_name", "moth::packer")
        self.cpp_info.libs = ["moth_packer"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
        if self.options.with_ui:
            # Gates the declarations in packer.h too, so consumers need it.
            self.cpp_info.defines = ["MOTH_PACKER_HAS_UI"]
