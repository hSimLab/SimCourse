add_library(memory INTERFACE)
target_include_directories(memory INTERFACE include)
target_link_libraries(memory INTERFACE isa)
