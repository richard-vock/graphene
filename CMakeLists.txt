cmake_minimum_required(VERSION 3.8.1)
project(graphene)

set (graphene_VERSION_MAJOR 0)
set (graphene_VERSION_MINOR 1)

set (OpenGL_GL_PREFERENCE "GLVND")

configure_file (
  "${PROJECT_SOURCE_DIR}/config.hpp.in"
  "${PROJECT_SOURCE_DIR}/include/graphene/config.hpp"
)

include_directories("${PROJECT_SOURCE_DIR}/include")
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

option(DEBUG_VERBOSE "Verbose debugging output" OFF)
if (${DEBUG_VERBOSE})
    add_definitions(-DDEBUG_VERBOSE)
endif()

add_compile_options(-Wall -Wextra -Werror -Wno-maybe-uninitialized -fPIC -g -DIMGUI_IMPL_OPENGL_LOADER_GLEW)

add_library(graphene
    "src/application.cpp"
    "src/boolean.cpp"
    "src/button.cpp"
    "src/camera.cpp"
    "src/choice.cpp"
    "src/choose_file.cpp"
    "src/imgui.cpp"
    "src/integer.cpp"
    "src/lines.cpp"
    "src/orbit_camera_model.cpp"
    "src/point_visibility.cpp"
    "src/range.cpp"
    "src/real.cpp"
    "src/renderer.cpp"
    "src/rgb.cpp"
    "src/rgba.cpp"
    "src/section.cpp"
    "src/text.cpp"
    "src/window.cpp"
)
target_compile_features(graphene PUBLIC cxx_std_17)
target_link_libraries(graphene CONAN_PKG::baldr CONAN_PKG::boost CONAN_PKG::eigen CONAN_PKG::fmt CONAN_PKG::glfw CONAN_PKG::glew CONAN_PKG::imgui "GL" "stdc++fs" "png")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "glsl/*.frag"
    "glsl/*.vert"
    )
foreach(GLSL_SOURCE_FILE ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL_SOURCE_FILE} NAME)
    #set(SPIRV "${CMAKE_INSTALL_PREFIX}/share/graphene/shaders/${FILE_NAME}")
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
        COMMAND glslangValidator -G ${GLSL_SOURCE_FILE} -o ${SPIRV}
        DEPENDS ${GLSL_SOURCE_FILE})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL_SOURCE_FILE)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)
add_dependencies(graphene Shaders)
install (DIRECTORY "${PROJECT_BINARY_DIR}/shaders" DESTINATION share/graphene)
