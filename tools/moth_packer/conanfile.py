from conan import ConanFile
from conan.tools.cmake import cmake_layout


class MothPackerCli(ConanFile):
    name = "moth_packer_cli"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        # with_ui: the CLI exposes --layout/--layouts-dir, which are the module's
        # moth::ui-backed collectors.
        self.requires("moth_packer/[~2.0]", options={"with_ui": True})
        self.requires("cli11/2.4.2")
        self.requires("fmt/[~10.2]")
        self.requires("spdlog/[~1.14]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)
