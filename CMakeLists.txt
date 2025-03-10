cmake_minimum_required(VERSION 3.1)
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
# target_compile_options(kakac
#     PRIVATE -D COMPILER
# )
