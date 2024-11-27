set(NAME naive_interpreter)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")

add_executable(${NAME} sim.cc)
target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/test)
target_link_libraries(${NAME} PRIVATE hart)
