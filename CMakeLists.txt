cmake_minimum_required(VERSION 3.19)

project(sim-course)

include(cmake/CPM.cmake)

add_subdirectory(third_party)
add_subdirectory(lib)
set(DIRS naive_interpreter
         inline_assembly
         asmjit_assembly
         jit_translator)

foreach(DIR ${DIRS})
    add_subdirectory(${DIR})
endforeach()

add_subdirectory(test)
