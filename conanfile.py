from conan import ConanFile


class MothToolkitSuperbuild(ConanFile):
    """Third-party dependency provisioner for the source superbuild.

    The modules are built from source via ``add_subdirectory`` in the top-level
    CMakeLists. Conan's only job here is to make the third-party libraries
    (which are not installed on the host) findable by generating CMakeDeps
    config files and a CMake toolchain. System libraries (GLFW, FreeType,
    HarfBuzz, Vulkan) are deliberately NOT declared here — they resolve via
    pkg-config / CMake's built-in find modules, matching the standalone module
    recipes.
    """

    settings = "os", "compiler", "build_type", "arch"

    # No name/version: this recipe is a local dev convenience, not a package.
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        # Union of the third-party deps across core/gfx/ui/bridge.
        self.requires("nlohmann_json/[>=3.11 <4]", transitive_headers=True)
        self.requires("spdlog/[~1.14]", transitive_headers=True)
        self.requires("fmt/[>=10.2 <13]", transitive_headers=True)
        self.requires("magic_enum/[~0.8]", transitive_headers=True)
        self.requires("range-v3/[~0.12]", transitive_headers=True)
        self.requires("vulkan-memory-allocator/3.0.1", transitive_headers=True)
        self.requires("entt/[~3.13]", transitive_headers=True)
        self.requires("box2d/2.4.1", transitive_headers=True)
        self.requires("zlib/1.3.2")
