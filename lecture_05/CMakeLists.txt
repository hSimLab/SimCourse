cmake_minimum_required(VERSION 3.16)

project(toy-sim)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")

add_executable(${PROJECT_NAME} sim.cc)
