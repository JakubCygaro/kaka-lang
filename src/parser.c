#include "parser.h"
#include <stdio.h>


Instruction* parse_from_file(FILE* source_file, size_t* instruction_amount){
    instructions = NULL;
    instructions_size = 0;

    token_stream = get_token_stream(source_file);

    Token* t;
    while((t = TokenStream_gett(token_stream)) != NULL){
        switch (t->t_type) {
        case COMMAND:
            Instruction i = parse_instruction(t->c_type);
            add_instruction(i);
        continue;

        default:
            err_print("invalid token, line: %lld", t->line);
        }
    }
    TokenStream_free(token_stream);
    *instruction_amount = instructions_size;
    return instructions;
}

static Instruction parse_instruction(COMMAND_TYPE c_type){
    Instruction i;
    Token* next_t;
    switch (c_type) {
        case PUSH:
            i.c_type = PUSH;
            next_t = TokenStream_gett(token_stream);
            ensure_token(VALUE, next_t);
            ensure_value_not(NONE, &next_t->v);
            i.v = next_t->v;
            break;
        case POP:
            i.c_type = POP;
            Value none_v;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case PRINT:
            i.c_type = PRINT;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case PRINT_STACK:
            i.c_type = PRINT_STACK;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case CAST_INT:
            i.c_type = CAST_INT;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case CAST_DOUBLE:
            i.c_type = CAST_DOUBLE;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case LABEL:
            i.c_type = LABEL;
            next_t = TokenStream_gett(token_stream);
            ensure_token(VALUE, next_t);
            ensure_value(STRING, &next_t->v);
            i.v = next_t->v;
            break;
        case JUMP:
            i.c_type = JUMP;
            next_t = TokenStream_gett(token_stream);
            ensure_token(VALUE, next_t);
            ensure_value(STRING, &next_t->v);
            i.v = next_t->v;
            break;
        case ADD:
            i.c_type = ADD;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case SUB:
            i.c_type = SUB;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case MUL:
            i.c_type = MUL;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case DIV:
            i.c_type = DIV;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case MOD:
            i.c_type = MOD;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case CMP:
            i.c_type = CMP;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case GREAT:
            i.c_type = GREAT;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case GREATEQ:
            i.c_type = GREATEQ;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case LESS:
            i.c_type = LESS;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case LESSEQ:
            i.c_type = LESSEQ;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case IF:
            i.c_type = IF;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        case CLONE:
            i.c_type = CLONE;
            none_v.v_type = NONE;
            none_v.none = NULL;
            i.v = none_v;
            break;
        default:
            err_print("command WIP");
    }
    return i;
}
static void add_instruction(Instruction i){
    instructions = realloc(instructions, sizeof i * ++instructions_size);
    instructions[instructions_size-1] = i;
}
static void ensure_token(TOKEN_TYPE type, Token* t){
    if (t == NULL)
        err_print("premature end of input");
    else if (t->t_type != type)
        err_print("invalid token, line: %lld", t->line);
}
static void ensure_value(VALUE_TYPE type, Value* v){
    if (v->v_type != type)
        err_print("improper value");
}
static void ensure_value_not(VALUE_TYPE type, Value* v){
    if (v->v_type == type)
        err_print("improper value");
}