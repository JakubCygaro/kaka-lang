#ifndef TYPES_H
#define TYPES_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef enum {
    VALUE,
    COMMAND,
    EMPTY
} TOKEN_TYPE;
typedef enum {
    INT,
    STRING,
    DOUBLE,
    NONE,
} VALUE_TYPE;

typedef struct {
    VALUE_TYPE v_type;
    union val {
        long long i;
        char* string;
        void* none;
        double d;
    };
} Value;

void Value_print(Value*);


typedef enum {
    PUSH,
    POP,
    ADD,
    SUB,
    PRINT,
    MUL,
    DIV,
    MOD,
    CMP,
    GREAT,
    GREATEQ,
    LESS,
    LESSEQ,
    PRINT_STACK,
    CAST_INT,
    CAST_DOUBLE,
    LABEL,
    JUMP,
    IF,
    CLONE,
    AND,
    OR,
    NOT,
    ASSERT,
    SWAP
} COMMAND_TYPE;

typedef struct {
    COMMAND_TYPE c_type;
    Value v;
    size_t line;
    size_t col;
    size_t jump_dest;
} Instruction;

typedef struct {
    TOKEN_TYPE t_type;
    size_t line;
    size_t col;
    union value {
        Value v;
        COMMAND_TYPE c_type;
    };
} Token;

typedef struct {
    size_t position;
    Token* tokens;
    size_t size;
} TokenStream;

void TokenStream_free(TokenStream*);
Token* TokenStream_gett(TokenStream*);
void TokenStream_ungett(TokenStream*);
void Token_print(Token*);
void CommandType_print(COMMAND_TYPE);

#endif