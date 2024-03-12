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
    CALL((BUILD " print.kaka"));


#define CALL(STR) assert(SUCCESS(STR))

#define INTERPRETER ".\\build\\kaka.exe"
#define COMPILER ".\\build\\kakac.exe"

void test_interpreter();

int main(){
    test_interpreter();
    
    printf("All tests passed\n");
    return 0;
}

void test_interpreter() {
    //build interpreter
    assert(SUCCESS("gcc ..\\src\\*.c -o .\\build\\kaka.exe"));

#define BUILD INTERPRETER

    FOREACH_SOURCE(BUILD);

#undef BUILD
}