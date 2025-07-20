from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps

class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("quill/10.0.1")
        self.requires("tomlplusplus/3.4.0")
        # self.requires("assimp/5.4.3")
        # self.requires("glm/1.0.1")      

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()