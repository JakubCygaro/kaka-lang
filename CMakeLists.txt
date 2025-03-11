cmake_minimum_required(VERSION 3.1)
enable_testing()
project(Kaka)

# interpreter
add_executable(kaka 
    src/main.c
    src/args_parse.c
    src/errors.c
    src/label.c
    src/lexer.c
    src/parser.c
    src/stringlist.c
    src/stringmap.c
    src/types.c
    src/util.c
)
target_include_directories(kaka PRIVATE ${CMAKE_SOURCE_DIR}/include)
# target_link_libraries(kaka -lmingw32)
# compiler
add_executable(kakac
    src/main.c
    src/args_parse.c
    src/errors.c
    src/label.c
    src/lexer.c
    src/parser.c
    src/stringlist.c
    src/stringmap.c
    src/types.c
    src/util.c
)
target_include_directories(kakac PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(kakac
    PUBLIC COMPILER=1
)
set(testDir "tests")
set(kakaTestSources
    "arth.kaka"
    "cast.kaka"
    "cmp.kaka"
    "jmp.kaka"
    "log.kaka"
    "misc.kaka"
    "print.kaka"
)
message(STATUS "Copying kaka source tests")
file(
    GLOB "*.kaka"
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/${testDir}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${testDir}
)
foreach(testSrc IN LISTS kakaTestSources)
    add_test(
        NAME "${testSrc}.interpreter.test"
        COMMAND kaka ${testSrc}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${testDir} 
    )
endforeach()
# add_subdirectory(tests)
