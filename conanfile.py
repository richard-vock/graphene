from conans import ConanFile, CMake, tools
import os


class GrapheneConan(ConanFile):
    name = "graphene"
    requires = (("eigen/3.3.9"),
                ("glew/2.1.0"),
                ("baldr/0.3"),
                ("fmt/7.1.3", "private"),
                ("glfw/3.2.1@bincrafters/stable", "private"),
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
    exports_sources = "!include/graphene/config.hpp", "cmake*", "include*", "src*", "glsl*", "CMakeLists.txt", "config.hpp.in"

    def build(self):
        cmake = CMake(self)
        cmake.definitions["DEBUG_VERBOSE"] = self.options.verbose
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("include/*.hpp", dst="include/graphene", keep_path=False)
        self.copy("include/*.ipp", dst="include/graphene", keep_path=False)
        self.copy("shaders/*.vert", dst="share")
        self.copy("shaders/*.frag", dst="share")
        self.copy("shaders/*.comp", dst="share")
        self.copy("*graphene.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["graphene"]

