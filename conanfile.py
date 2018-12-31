from conans import ConanFile, CMake, tools


class GrapheneConan(ConanFile):
    name = "graphene"
    requires = (("boost/1.68.0@conan/stable"),
                ("eigen/3.3.5@conan/stable"),
                ("fmt/5.2.1@bincrafters/stable"),
                ("baldr/0.1@richard-vock/dev", "private"),
                ("glfw/3.2.1@bincrafters/stable", "private"),
                ("glew/2.1.0@bincrafters/stable", "private"),
                ("imgui/1.62@bincrafters/stable", "private"))
    version = "0.1"
    license = "unlicense"
    author = "Richard Vock vock@cs.uni-bonn.de"
    url = "https://github.com/richard-vock/graphene"
    description = "3D Visualization Prototyping Library"
    topics = ("rendering", "visualization", "pointclouds", "meshes")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "verbose": [True, False]}
    default_options = "shared=False", "verbose=False"
    generators = "cmake"
    exports_sources = "!include/graphene/config.hpp", "include*", "src*", "glsl*", "CMakeLists.txt", "config.hpp.in"

    def build(self):
        cmake = CMake(self)
        cmake.definitions["DEBUG_VERBOSE"] = self.options.verbose
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("include/graphene/*.hpp")
        self.copy("include/graphene/*.ipp")
        self.copy("shaders/*.vert", dst="share")
        self.copy("shaders/*.frag", dst="share")
        self.copy("*graphene.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["graphene"]

