function(compile_targets)
    set(OneValueArgs LIBRARY PROGRAM)
    set(MultiValueArgs SOURCES DEPENDENCIES SHADERS)
    cmake_parse_arguments(COMPILE_TARGETS "" "${OneValueArgs}" "${MultiValueArgs}" ${ARGN})

    if (COMPILE_TARGETS_LIBRARY)
        message(STATUS "Compiling library ${COMPILE_TARGETS_LIBRARY}")
        add_library(${COMPILE_TARGETS_LIBRARY} "${COMPILE_TARGETS_SOURCES}")
        target_compile_features(${COMPILE_TARGETS_LIBRARY} PUBLIC cxx_std_17)
        if (${DEV})
            target_compile_definitions(${COMPILE_TARGETS_LIBRARY} PRIVATE -DEIGEN_DONT_VECTORIZE)
        endif()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_link_libraries(${COMPILE_TARGETS_LIBRARY} ${COMPILE_TARGETS_DEPENDENCIES})
        else()
            target_link_libraries(${COMPILE_TARGETS_LIBRARY} ${COMPILE_TARGETS_DEPENDENCIES} pthread)
        endif()
        add_dependencies(${COMPILE_TARGETS_LIBRARY} ${COMPILE_TARGETS_SHADERS})
    endif()
    if (COMPILE_TARGETS_PROGRAM)
        message(STATUS "Compiling executable ${COMPILE_TARGETS_PROGRAM}")
        add_executable(${COMPILE_TARGETS_PROGRAM} ${COMPILE_TARGETS_SOURCES})
        target_compile_features(${COMPILE_TARGETS_PROGRAM} PUBLIC cxx_std_17)
        if (${DEV})
            target_compile_definitions(${COMPILE_TARGETS_PROGRAM} PRIVATE -DEIGEN_DONT_VECTORIZE)
        endif()
        target_link_libraries(${COMPILE_TARGETS_PROGRAM} ${COMPILE_TARGETS_DEPENDENCIES})
        add_dependencies(${COMPILE_TARGETS_PROGRAM} ${COMPILE_TARGETS_SHADERS})
    endif()
endfunction()
