set(DIRS cpu_state isa memory decoder logger hart timer)

foreach(DIR ${DIRS})
  add_subdirectory(${DIR})
endforeach()
