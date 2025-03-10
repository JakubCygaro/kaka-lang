cmake_minimum_required(VERSION 3.1)
project(Kaka)

set(testDir "kaka_src")
add_executable(kaka_test
    tests.c
)
target_compile_definitions(kaka_test
    PUBLIC COMPILER=$<TARGET_FILE:kakac>
    PUBLIC INTERPRETER=$<TARGET_FILE:kaka>
)
message(STATUS "Copying kaka source tests")
file(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/${testDir}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${testDir}
)
message(STATUS "Running tests")
add_test(
    NAME kakaTest
    COMMAND kaka_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${testDir} 
)
