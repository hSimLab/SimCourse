cmake_minimum_required(VERSION 3.16)

project(sim-course)

include(cmake/CPM.cmake)

add_subdirectory(third_party)

set(DIRS naive_interpreter
         inline_assembly
         asmjit_assembly
         jit_translator)

foreach(DIR ${DIRS})
    add_subdirectory(${DIR})
endforeach()

add_subdirectory(test)
