set(DIRS cpu_state isa memory decoder hart)

foreach(DIR ${DIRS})
  add_subdirectory(${DIR})
endforeach()
