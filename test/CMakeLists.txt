add_custom_target(
    RunTestGen ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Running Test Gen script"
    COMMAND python3 ${CMAKE_SOURCE_DIR}/test/testgen.py --iter_num 10000000
    WORKING_DIRECTORY ${CMAKE_CUR_SOURCE_DIR}
    COMMENT "Generating bytecode & regfile initialization"
)
