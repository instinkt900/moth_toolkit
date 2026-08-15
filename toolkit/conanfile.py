from conan import ConanFile
from conan.errors import ConanInvalidConfiguration


class MothToolkit(ConanFile):
    name = "moth_toolkit"

    license = "MIT"
    url = "https://github.com/instinkt900/moth_toolkit"
    description = "Aggregator meta-package for the Moth toolkit. Pulls the enabled modules and exposes the MOTH_ENABLE_* feature flags."

    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "version.txt", "CMakeLists.txt"

    options = {
        "enable_core": [True, False],
        "enable_gfx": [True, False],
        "enable_ui": [True, False],
        "enable_bridge": [True, False],
    }
    default_options = {
        "enable_core": True,
        "enable_gfx": True,
        "enable_ui": True,
        "enable_bridge": True,
    }

    def set_version(self):
        if not self.version:
            from conan.tools.files import load
            self.version = load(self, "version.txt").strip()

    def validate(self):
        if self.options.enable_bridge and not (self.options.enable_core and self.options.enable_gfx and self.options.enable_ui):
            raise ConanInvalidConfiguration(
                "moth::bridge requires core, gfx, and ui — enable those options or set enable_bridge=False"
            )
        if self.options.enable_gfx and not self.options.enable_core:
            raise ConanInvalidConfiguration(
                "moth::gfx requires moth::core — enable_core or disable gfx"
            )
        if self.options.enable_ui and not self.options.enable_core:
            raise ConanInvalidConfiguration(
                "moth::ui requires moth::core — enable_core or disable ui"
            )

    def requirements(self):
        if self.options.enable_core:
            self.requires("moth_core/0.1.0")
        if self.options.enable_gfx:
            self.requires("moth_graphics/1.2.0")
        if self.options.enable_ui:
            self.requires("moth_ui/1.1.2")
        if self.options.enable_bridge:
            self.requires("moth_bridge/0.1.0")

    def package_info(self):
        # Pure aggregator — no headers or libraries of its own.
        self.cpp_info.includedirs = []
        self.cpp_info.libdirs = []

        # Feature flags, mirrored in cmake/features.h.in for the source superbuild.
        self.cpp_info.defines = [
            "MOTH_ENABLE_CORE={}".format(1 if self.options.enable_core else 0),
            "MOTH_ENABLE_GFX={}".format(1 if self.options.enable_gfx else 0),
            "MOTH_ENABLE_UI={}".format(1 if self.options.enable_ui else 0),
            "MOTH_ENABLE_BRIDGE={}".format(1 if self.options.enable_bridge else 0),
        ]

        # Expose each enabled module as a transitive dependency so a consumer that
        # links the single `moth_toolkit::moth_toolkit` target pulls them all in.
        self.cpp_info.requires = []
        if self.options.enable_core:
            self.cpp_info.requires.append("moth_core::moth_core")
        if self.options.enable_gfx:
            self.cpp_info.requires.append("moth_graphics::moth_graphics")
        if self.options.enable_ui:
            self.cpp_info.requires.append("moth_ui::moth_ui")
        if self.options.enable_bridge:
            self.cpp_info.requires.append("moth_bridge::moth_bridge")
