#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define SUCCESS(CALL) system(CALL) == 0
#define FAILURE(CALL) system(CALL) != 0

#define FOREACH_SOURCE(BUILD)\
    CALL((BUILD " arth.kaka"));\
    CALL((BUILD " log.kaka"));\
    CALL((BUILD " cmp.kaka"));\
    CALL((BUILD " cast.kaka"));\
    CALL((BUILD " jmp.kaka"));\
    CALL((BUILD " print.kaka"));\
    CALL((BUILD " misc.kaka"));

#define CALL(STR) assert(SUCCESS(STR))

#define INTERPRETER ".\\build\\kaka.exe"
#define COMPILER ".\\build\\kakac.exe"

void test_interpreter();
void test_compiler();

int main(){
    test_interpreter();
    //test_compiler();
    printf("All tests passed\n");
    return 0;
}

void test_interpreter() {
    //build interpreter
    printf("Compiling the interpreter...\n");
    assert(SUCCESS("gcc ..\\src\\*.c -o .\\build\\kaka.exe"));
    printf("Testing sources...\n");
#define BUILD INTERPRETER

    FOREACH_SOURCE(BUILD);

#undef BUILD
    printf("Interpreter tests done.\n");
}

void test_compiler(){
    // build compiler
    printf("Compiling the compiler...\n");
    assert(SUCCESS("gcc -D COMPILER ..\\src\\*.c -o .\\build\\kakac.exe"));
    printf("Testing sources...\n");
#define BUILD COMPILER

    FOREACH_SOURCE(BUILD);

#undef BUILD
    printf("Compiler tests done.\n");
}