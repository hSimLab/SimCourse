set(DIRS cpu_state isa memory decoder)

foreach(DIR ${DIRS})
  add_subdirectory(${DIR})
endforeach()
