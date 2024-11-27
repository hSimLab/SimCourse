add_library(decoder STATIC decoder.cc)
target_include_directories(decoder PUBLIC include/)
target_link_libraries(decoder PUBLIC isa)
