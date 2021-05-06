find_program(glslc NAMES glslc DOC "glsl to SPIR-V compiler")
if(NOT glslc)
   message(FATAL_ERROR "failed to find glslc")
endif()

function(compile_shaders)
    cmake_parse_arguments(COMPILE_SHADERS "" "" "SOURCES" ${ARGN})
    file(GLOB_RECURSE SHADER_SOURCE_FILES ${COMPILE_SHADERS_SOURCES})

    set(SHADER_OUTPUT_FOLDER "shaders/")

    message(STATUS "Installing compiled shaders in ${SHADER_OUTPUT_FOLDER}")

    foreach(SOURCE_FILE ${SHADER_SOURCE_FILES})
        get_filename_component(BASE_NAME ${SOURCE_FILE} NAME_WE)
        get_filename_component(FILE_NAME ${SOURCE_FILE} NAME)
        message(STATUS "Compiling shader ${BASE_NAME}")
        set(COMPILED_FILE "${SHADER_OUTPUT_FOLDER}/${FILE_NAME}")
        add_custom_command(
            OUTPUT "${COMPILED_FILE}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_FOLDER}"
            COMMAND ${glslc} ARGS -I ${PROJECT_SOURCE_DIR}/include ${SOURCE_FILE} -o "${COMPILED_FILE}" --target-env=opengl4.5
            DEPENDS ${SOURCE_FILE})
        list(APPEND COMPILED_FILES ${COMPILED_FILE})
    endforeach(SOURCE_FILE)

    add_custom_target(
        ${ARGV0}
        DEPENDS ${COMPILED_FILES}
    )
endfunction()
