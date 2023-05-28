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
    PRINT_STACK,
    CAST_INT,
    CAST_DOUBLE,
    LABEL,
    JUMP,
    IF,
    CLONE,
} COMMAND_TYPE;

typedef struct {
    COMMAND_TYPE c_type;
    Value v;
    size_t jump_dest;
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

//#define TABLE_SIZE 100

// typedef struct {
//     char* key;
//     size_t value;
// } LabelNode;

// typedef struct {
//     LabelNode** nodes;
//     size_t size;
// } LabelMap;

// // unsigned long hash(const char* str);
// // unsigned long index_for(unsigned long hash);

// // LabelMap LabelMap_create();
// // void LabelMap_free(LabelMap*);


// int LabelMap_put(LabelMap*, char*, size_t);
// LabelNode* LabelMap_get(LabelMap*, char*, size_t);
// void LabelMap_free(LabelMap*);
// LabelMap LabelMap_new();
// int LabelMap_check(LabelMap*, LabelNode);

void TokenStream_free(TokenStream*);
Token* TokenStream_gett(TokenStream*);
void TokenStream_ungett(TokenStream*);
void Token_print(Token*);
void CommandType_print(COMMAND_TYPE);

#endif