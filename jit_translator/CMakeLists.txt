set(NAME jit_translator)

set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

add_executable(${NAME} sim.cc)

target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/asmjit/src)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/test)
target_link_libraries(${NAME} PRIVATE asmjit)
