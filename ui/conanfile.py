from conan import ConanFile
from conan.tools.cmake import cmake_layout

class MothUI(ConanFile):
	name = "moth_ui"
	version = "0.1"
	settings = "os", "compiler", "build_type", "arch"
	generators = "CMakeToolchain", "CMakeDeps"

	def requirements(self):
		self.requires("nlohmann_json/3.11.2")
		self.requires("magic_enum/0.7.3")
		self.requires("range-v3/0.12.0")
		self.requires("fmt/10.0.0")

	def layout(self):
		cmake_layout(self)