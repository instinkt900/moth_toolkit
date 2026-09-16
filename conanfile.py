from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout


class MothToolkitSuperbuild(ConanFile):
    """Third-party dependency provisioner for the source superbuild.

    The modules are built from source via ``add_subdirectory`` in the top-level
    CMakeLists. Conan's only job here is to make the third-party libraries
    findable by generating CMakeDeps config files and a CMake toolchain.

    System libraries (GLFW, FreeType, HarfBuzz, Vulkan) are deliberately not
    declared here on Linux: they resolve via pkg-config and CMake's built-in
    find modules, matching what the standalone module recipes do there.

    The enable_* options mirror the MOTH_ENABLE_* CMake options one for one, and
    each dependency is required only when a module that uses it is enabled.
    Without that, every configuration provisioned the union of all dependencies:
    a core-only build still fetched Box2D, miniaudio, asio and FastNoise2. On
    Linux that only cost time, because ConanCenter has prebuilt binaries; on
    Windows there are none for the toolchain CI runs, so each one is built from
    source and a configuration that does not use a library still paid for it --
    and still failed if that library's recipe was broken.

    The defaults match the CMake options, so `conan install .` with no options
    provisions everything, as before. Passing an option the CMake configure does
    not match is safe in one direction only: a dependency provisioned but unused
    is harmless, while one omitted for a module that is enabled is a configure
    error, so the options default on.
    """

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "enable_core": [True, False],
        "enable_gfx": [True, False],
        "enable_ui": [True, False],
        "enable_bridge": [True, False],
        "enable_ecs": [True, False],
        "enable_physics": [True, False],
        "enable_tilemap": [True, False],
        "enable_audio": [True, False],
        "enable_assets": [True, False],
        "enable_anim": [True, False],
        "enable_net": [True, False],
        "enable_noise": [True, False],
        "enable_profile": [True, False],
        "enable_packer": [True, False],
        "enable_toolkit": [True, False],
        "enable_tools": [True, False],
        "enable_examples": [True, False],
        "enable_glslang": [True, False],
    }
    default_options = {
        "enable_core": True,
        "enable_gfx": True,
        "enable_ui": True,
        "enable_bridge": True,
        "enable_ecs": True,
        "enable_physics": True,
        "enable_tilemap": True,
        "enable_audio": True,
        "enable_assets": True,
        "enable_anim": True,
        "enable_net": True,
        "enable_noise": True,
        "enable_profile": True,
        "enable_packer": True,
        "enable_toolkit": True,
        # MOTH_ENABLE_TOOLS and MOTH_ENABLE_EXAMPLES default off in CMake.
        "enable_tools": False,
        "enable_examples": False,
        "enable_glslang": False,
    }

    # No name/version: this recipe is a local dev convenience, not a package.
    generators = "CMakeToolchain", "CMakeDeps"

    def layout(self):
        # Redirect generated toolchain/deps into build/<build_type>/generators
        # (and emit the conan-release preset) so `conan install .` behaves like
        # the standalone module recipes instead of dumping files in the root.
        cmake_layout(self)

    def configure(self):
        # The superbuild consumes fastnoise2 directly, so it needs the same
        # Windows workaround as the moth_noise recipe: FastNoise2's CMake installs
        # a PDB directory that a static MSVC build never creates. See
        # modules/noise/conanfile.py.
        if self.settings.os == "Windows" and self.options.enable_noise:
            self.options["fastnoise2/*"].shared = True

    def validate(self):
        # Every module is C++17. Checked here so a profile below it -- MSVC's
        # autodetected default is 14 -- fails with one clear message instead of a
        # validation error from each dependency that also needs 17.
        check_min_cppstd(self, 17)

    def _enabled(self, *modules):
        """True when any of the named modules is enabled."""
        return any(bool(getattr(self.options, "enable_" + m)) for m in modules)

    def requirements(self):
        # Which module needs what is taken from the standalone module recipes;
        # keep the two in step when a module gains or drops a dependency.

        # moth::core math types serialise to JSON, and so do the layout, asset,
        # tilemap, net, noise and packer formats.
        if self._enabled("core", "ui", "anim", "assets", "net", "noise",
                         "packer", "tilemap"):
            self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)

        # The moth::core logging facade formats via fmt and routes to spdlog.
        # spdlog 1.14 pins fmt to 10.2, so keep fmt on that line.
        if self._enabled("core", "ui", "packer"):
            self.requires("spdlog/[~1.14]", transitive_headers=True)
            self.requires("fmt/[~10.2]", transitive_headers=True)

        if self._enabled("ui"):
            self.requires("magic_enum/[~0.8]", transitive_headers=True)
        if self._enabled("ui", "packer"):
            self.requires("range-v3/[~0.12]", transitive_headers=True)
        if self._enabled("gfx"):
            self.requires("vulkan-memory-allocator/3.0.1", transitive_headers=True)
        if self._enabled("ecs"):
            self.requires("entt/[~3.15]", transitive_headers=True)
        if self._enabled("physics"):
            self.requires("box2d/2.4.1", transitive_headers=True)
        if self._enabled("tilemap"):
            self.requires("zlib/1.3.2")
        if self._enabled("audio"):
            self.requires("miniaudio/0.11.18", transitive_headers=True)
        if self._enabled("net"):
            self.requires("asio/1.30.2", transitive_headers=True)
        # Node-graph noise generation plus the metadata system moth::noise
        # serialises against.
        if self._enabled("noise"):
            self.requires("fastnoise2/1.1.1", transitive_headers=True)
        # moth::packer: image load/write and bin packing, plus glob expansion
        # for its path collectors.
        if self._enabled("packer"):
            self.requires("stb/cci.20240531")
            self.requires("p-ranav-glob/0.0.1")
        # CLI11 is for the moth_packer/moth_pak front ends.
        if self._enabled("tools"):
            self.requires("cli11/2.4.2")

        # On Windows there is no system package manager to resolve these, so the
        # superbuild takes them from Conan exactly as the standalone core and
        # gfx recipes do. Without them the configure fails at core's
        # find_package(glfw3), which is what the CMake comment there means by
        # "Conan on Windows".
        if self.settings.os == "Windows":
            if self._enabled("core", "gfx"):
                self.requires("glfw/3.3.8", transitive_headers=True)
            if self._enabled("gfx"):
                # freetype is pinned rather than ranged: harfbuzz 8.3.0 requires
                # exactly 2.13.2, and [~2.13] resolves to 2.13.3, which is a hard
                # version conflict in the graph.
                self.requires("freetype/2.13.2", transitive_headers=True)
                self.requires("harfbuzz/[~8.3]", transitive_headers=True)
                # find_package(Vulkan) resolves to these rather than a system SDK.
                self.requires("vulkan-headers/1.3.243.0", transitive_headers=True)
                self.requires("vulkan-loader/1.3.243.0")

        # Runtime GLSL compilation for custom shaders (opt-in; pairs with the
        # MOTH_GRAPHICS_ENABLE_GLSLANG CMake option).
        if self.options.enable_glslang and self._enabled("gfx"):
            self.requires("glslang/1.3.268.0")
            # glslang's public SPIR-V headers include spirv-tools/libspirv.h.
            self.requires("spirv-tools/1.3.268.0")
