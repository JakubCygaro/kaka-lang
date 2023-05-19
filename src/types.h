#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
// typedef enum {
    
// } INSTRUCTION_TYPE;
typedef enum {
    VALUE,
    COMMAND,
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
        int i;
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
} COMMAND_TYPE;

typedef struct {
    COMMAND_TYPE c_type;
    Value v;
} Instruction;

typedef struct {
    TOKEN_TYPE t_type;
    size_t line;
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