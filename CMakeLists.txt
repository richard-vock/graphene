cmake_minimum_required(VERSION 3.8.1)
project(graphene)

set (graphene_VERSION_MAJOR 0)
set (graphene_VERSION_MINOR 1)
set (OpenGL_GL_PREFERENCE "GLVND")

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(CompileShaders)
include(CompileTargets)

include_directories("${PROJECT_SOURCE_DIR}/include")
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.hpp.in"
  "${PROJECT_SOURCE_DIR}/include/config.hpp"
)

option(DEBUG_VERBOSE "Verbose debugging output" OFF)
if (${DEBUG_VERBOSE})
    add_definitions(-DDEBUG_VERBOSE)
endif()

add_compile_options(-Wall -Wextra -O2 -fPIC -g -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DEIGEN_DONT_VECTORIZE -DEIGEN_MAX_ALIGN_BYTES=0 -DEIGEN_MAX_STATIC_ALIGN_BYTES=0 -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT)

find_package(Boost COMPONENTS signals2)

set(DEPENDENCIES CONAN_PKG::baldr
                 ${Boost_LIBRARIES}
                 CONAN_PKG::eigen
                 CONAN_PKG::fmt
                 CONAN_PKG::glfw
                 CONAN_PKG::glew
                 CONAN_PKG::imgui
                 GL
                 stdc++fs
                 png)

compile_shaders(shaders SOURCES "src/*.comp" "src/*.frag" "src/*.vert")

compile_targets(
    LIBRARY graphene
    SOURCES "src/application.cpp"
            "src/renderable.cpp"
            "src/boolean.cpp"
            "src/box.cpp"
            "src/button.cpp"
            "src/camera.cpp"
            "src/choice.cpp"
            "src/choose_file.cpp"
            "src/cross.cpp"
            "src/depth_vis_pass.cpp"
            "src/imgui.cpp"
            "src/integer.cpp"
            "src/lines.cpp"
            "src/orbit_camera_model.cpp"
            "src/point_visibility.cpp"
            "src/primitive_renderer.cpp"
            "src/range.cpp"
            "src/real.cpp"
            "src/renderer.cpp"
            "src/rgb.cpp"
            "src/rgba.cpp"
            "src/section.cpp"
            "src/splat_renderer.cpp"
            "src/text.cpp"
            "src/trivial_point_renderer.cpp"
            "src/tonemap_pass.cpp"
            "src/window.cpp"
    SHADERS shaders
    DEPENDENCIES ${DEPENDENCIES})
#file(GLOB_RECURSE GLSL_SOURCE_FILES
#    "glsl/*.frag"
#    "glsl/*.vert"
#    "glsl/*.comp"
#    )
#foreach(GLSL_SOURCE_FILE ${GLSL_SOURCE_FILES})
#    get_filename_component(FILE_NAME ${GLSL_SOURCE_FILE} NAME)
#    #set(SPIRV "${CMAKE_INSTALL_PREFIX}/share/graphene/shaders/${FILE_NAME}")
#    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}")
#    add_custom_command(
#        OUTPUT ${SPIRV}
#        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
#        COMMAND glslangValidator -G ${GLSL_SOURCE_FILE} -o ${SPIRV}
#        DEPENDS ${GLSL_SOURCE_FILE})
#    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
#endforeach(GLSL_SOURCE_FILE)
#
#add_custom_target(
#    Shaders
#    DEPENDS ${SPIRV_BINARY_FILES}
#)
#add_dependencies(graphene Shaders)
#install (DIRECTORY "${PROJECT_BINARY_DIR}/shaders" DESTINATION share/graphene)
