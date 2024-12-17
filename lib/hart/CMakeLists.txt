add_library(hart INTERFACE)
target_include_directories(hart INTERFACE include)
target_include_directories(hart INTERFACE ${CMAKE_SOURCE_DIR}/test)
target_link_libraries(hart INTERFACE memory cpu_state decoder logger timer)
