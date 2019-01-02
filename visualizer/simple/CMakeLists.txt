cmake_minimum_required(VERSION 3.8.1)
project(simple)
include_directories("${PROJECT_SOURCE_DIR}/include")
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set (OpenGL_GL_PREFERENCE "GLVND")


add_compile_options(-Wall -Wextra -Werror -Wno-maybe-uninitialized -fPIC -g -DIMGUI_IMPL_OPENGL_LOADER_GLEW)

add_executable(simple
    "src/simple.cpp"
)
target_compile_features(simple PUBLIC cxx_std_17)
target_link_libraries(simple CONAN_PKG::baldr CONAN_PKG::graphene CONAN_PKG::graphene-pcl CONAN_PKG::CLI11)
