cmake_minimum_required(VERSION 3.16)

project(toy-sim)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_subdirectory(third_party)

add_executable(${PROJECT_NAME} sim.cc)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/asmjit/src)
target_link_libraries(${PROJECT_NAME} PRIVATE asmjit)
