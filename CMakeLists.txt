cmake_minimum_required(VERSION 3.5)
enable_testing()
project(Kaka)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

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
message(STATUS "Copying kaka source tests.")
file(
    COPY ${testDir}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

if(NOT WIN32)
    message(STATUS "Platform is not WIN32, compiler run tests will be skipped.")
endif()

add_test(
    NAME "interpreter.test"
    COMMAND kaka 
)
add_test(
    NAME "compiler.test"
    COMMAND kakac 
)
foreach(testSrc IN LISTS kakaTestSources)
    add_test(
        NAME "${testSrc}.interpreter.test"
        COMMAND kaka ${testSrc}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${testDir} 
    )
    set(outExe ${testSrc}.exe)
    add_test(
        NAME "${testSrc}.compiler.test"
        COMMAND kakac ${testSrc} ${outExe}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${testDir} 
    )
    if(WIN32)
        add_test(
            NAME "${testSrc}.compiler.run.test"
            COMMAND ${outExe}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${testDir} 
        )
    endif()
endforeach()


install(TARGETS kaka DESTINATION bin)
install(TARGETS kakac DESTINATION bin)
