from conan import ConanFile
from conan.tools.cmake import CMake

class MultorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("quill/10.0.1")
        self.requires("tomlplusplus/3.4.0")
        self.requires("assimp/6.0.2")
        self.requires("glm/1.0.1")
        self.requires("imgui/1.91.4-docking")    
        self.requires("sdl/3.4.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()