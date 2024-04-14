#include "parser.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "stringmap.h"

static Instruction parse_instruction(COMMAND_TYPE, size_t line, size_t col);
static void add_instruction(Instruction);
static void ensure_token(TOKEN_TYPE, Token*);
static void ensure_value(VALUE_TYPE, Value*);
static void ensure_value_not(VALUE_TYPE, Value*);
static int get_type_params();
static int verify_label(const char*);

Instruction* parse_from_file(FILE* source_file, size_t* instruction_amount, StringMap* string_map){
    instructions = NULL;
    instructions_size = 0;

    token_stream = get_token_stream(source_file, string_map);

    Token* t;
    while((t = TokenStream_gett(token_stream)) != NULL){
        switch (t->t_type) {
        case COMMAND:
            Instruction i = parse_instruction(t->c_type, t->line, t->col);
            i.line = t->line;
            i.col = t->col;
            add_instruction(i);
        continue;
        case VALUE:
            // if(t->v.v_type == STRING){
            //     StringMapNode node = StringMapNode_new((unsigned char*)t->v.string);
            //     int res = StringMap_insert(string_map, node);
            //     if(!res){
            //         const unsigned char *str = (unsigned char *)t->v.string; 
            //         t->v.string = (char*)StringMap_get(string_map, str);
            //         free((void*)str);
            //     }
            // }
            Instruction implicit_push = {
                PUSH,
                t->v,
                0,
            };
            i.line = t->line;
            i.col = t->col;
            add_instruction(implicit_push);
        continue;
        case EMPTY:
            continue;
        default:
            err_print("invalid token, (%lld:%lld)\n", t->line, t->col);
        }
    }
    TokenStream_free(token_stream);
    *instruction_amount = instructions_size;
    return instructions;
}

static Instruction parse_instruction(COMMAND_TYPE c_type, size_t line, size_t col){
    Instruction ins;
    Token* next_t;
    switch (c_type) {
        case PUSH:
            ins.c_type = PUSH;
            next_t = TokenStream_gett(token_stream);
            ensure_token(VALUE, next_t);
            ensure_value_not(NONE, &next_t->v);
            ins.v = next_t->v;
            break;
        case POP:
            ins.c_type = POP;
            Value none_v;
            none_v.v_type = NONE;
            none_v.none = NULL;
            ins.v = none_v;
            break;
        case PRINT:
            ins.c_type = PRINT;
            next_t = TokenStream_gett(token_stream);
            if(next_t != NULL){
                if(next_t->t_type == VALUE){
                    if(next_t->v.v_type == INT){
                        ins.v.v_type = INT;
                        ins.v.i = next_t->v.i;
                        break;
                    }
                }
                TokenStream_ungett(token_stream);
            }
            none_v.v_type = INT;
            none_v.i = 0;
            ins.v = none_v;
            break;
        case PRINT_STACK:
            ins.c_type = PRINT_STACK;
            none_v.v_type = NONE;
            none_v.none = NULL;
            ins.v = none_v;
            break;
        case CAST_INT:
            ins.c_type = CAST_INT;
            none_v.v_type = NONE;
            none_v.none = NULL;
            ins.v = none_v;
            break;
        case CAST_DOUBLE:
            ins.c_type = CAST_DOUBLE;
            none_v.v_type = NONE;
            none_v.none = NULL;
            ins.v = none_v;
            break;
        case LABEL:
            ins.c_type = LABEL;
            next_t = TokenStream_gett(token_stream);
            ensure_token(VALUE, next_t);
            ensure_value(STRING, &next_t->v);
            int res = verify_label(next_t->v.string);
            if(!res){
                err_print("Label string contains disallowed characters (%lld:%lld)", next_t->line, next_t->col);
            }
            ins.v = next_t->v;
            break;
        case JUMP:
            ins.c_type = JUMP;
            next_t = TokenStream_gett(token_stream);
            ensure_token(VALUE, next_t);
            ensure_value(STRING, &next_t->v);
            ins.v = next_t->v;
            break;
        case ADD:{
                ins.c_type = ADD;
                Value v = {
                    .v_type = INT,
                    .i = get_type_params(),
                };
                // none_v.v_type = INT;
                // none_v.i = get_type_params();
                ins.v = v;
            }break;
        case SUB:{
                ins.c_type = SUB;
                Value v = {
                    .v_type = INT,
                    .i = get_type_params(),
                };
                // none_v.v_type = NONE;
                // none_v.none = NULL;
                ins.v = v;
            }break;
        case MUL:{
                ins.c_type = MUL;
                Value v = {
                    .v_type = INT,
                    .i = get_type_params(),
                };
                // none_v.v_type = NONE;
                // none_v.none = NULL;
                ins.v = v;
            }break;
        case DIV:{
                ins.c_type = DIV;
                Value v = {
                        .v_type = INT,
                        .i = get_type_params(),
                };
                // none_v.v_type = NONE;
                // none_v.none = NULL;
                ins.v = v;
            }break;
        case MOD:{
                ins.c_type = MOD;
                Value v = {
                    .v_type = INT,
                    //.i = get_type_params(),
                };
                // none_v.v_type = NONE;
                // none_v.none = NULL;
                ins.v = v;
            }break;
        case CMP:{
                ins.c_type = CMP;
                Value v = {
                    .v_type = INT,
                    .i = get_type_params(),
                };
                // none_v.v_type = NONE;
                // none_v.none = NULL;
                ins.v = v;
            }break;
        case GREAT:{
                ins.c_type = GREAT;
                Value v = {
                    .v_type = INT,
                    .i = get_type_params(),
                };
                // none_v.v_type = NONE;
                // none_v.none = NULL;
                ins.v = v;
            }break;
        case GREATEQ:{
                ins.c_type = GREATEQ;
                Value v = {
                    .v_type = INT,
                    .i = get_type_params(),
                };
                // none_v.v_type = NONE;
                // none_v.none = NULL;
                ins.v = v;
            }break;
        case LESS:{
                ins.c_type = LESS;
                Value v = {
                    .v_type = INT,
                    .i = get_type_params(),
                };
                // none_v.v_type = NONE;
                // none_v.none = NULL;
                ins.v = v;
            }break;
        case LESSEQ:{
                ins.c_type = LESSEQ;
                Value v = {
                    .v_type = INT,
                    .i = get_type_params(),
                };
                // none_v.v_type = NONE;
                // none_v.none = NULL;
                ins.v = v;
            }break;
        case IF:{
                ins.c_type = IF;
                // Value v = {
                //     .v_type = INT,
                //
                // };
                none_v.v_type = NONE;
                none_v.none = NULL;
                ins.v = none_v;
            }break;
        case CLONE:{
                ins.c_type = CLONE;

                Token *next = TokenStream_gett(token_stream);
                Value count = {
                    .v_type = INT,
                    .i = 1,
                };
                if(next != NULL) {
                    if(next->t_type == VALUE) {
                        if(next->v.v_type == INT) {
                            count.i = next->v.i;
                        }
                    }
                }
                ins.v = count;
            }break;
        case AND:{
                ins.c_type = AND;
                // Value v = {
                //     .v_type = INT,
                //     //.i = get_type_params(),
                // };
                none_v.v_type = NONE;
                none_v.none = NULL;
                ins.v = none_v;
            }break;
        case OR:{
                ins.c_type = OR;
                // Value v = {
                //     .v_type = INT,
                //     //.i = get_type_params(),
                // };
                none_v.v_type = NONE;
                none_v.none = NULL;
                ins.v = none_v;
            }break;
        case NOT:{
                ins.c_type = NOT;
                // Value v = {
                //     .v_type = INT,
                //     //.i = get_type_params(),
                // };
                none_v.v_type = NONE;
                none_v.none = NULL;
                ins.v = none_v;
            }break;
        case ASSERT: {
                ins.c_type = ASSERT;
                Token *t = TokenStream_gett(token_stream);
                ensure_token(VALUE, t);
                ensure_value(STRING, &t->v);
                Value msg = {
                    .v_type = STRING,
                    .string = t->v.string
                };
                ins.v = msg;
        }break;
        case SWAP: {
                ins.c_type = SWAP;
        } break;
        default:
            err_print("command WIP");
    }
    return ins;
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
static int get_type_params(){
    int ret = PARAM_NONE;
    Token *t = TokenStream_gett(token_stream);
    if(t == NULL)
        return ret;
    if(t->t_type != COMMAND){
        TokenStream_ungett(token_stream);
        return ret;
    }

    if(t->c_type == CAST_INT)
        ret = PARAM_1_INT;
    else if(t->c_type == CAST_DOUBLE) {
        ret = PARAM_1_DOUBLE;
    }
    else {
        TokenStream_ungett(token_stream);
        return ret;
    }

    t = TokenStream_gett(token_stream);
    if(t == NULL)
        return ret;

    if(t->c_type == CAST_INT)
        ret |= PARAM_2_INT;
    else if(t->c_type == CAST_DOUBLE) {
        ret |= PARAM_2_DOUBLE;
    }
    else {
        TokenStream_ungett(token_stream);
        return ret;
    }
    return ret;
}

static int verify_label(const char* str) {
    char c;
    while((c = *str++)){
        if(!isalnum(c) && c != '_')
            return 0;
    }
    return 1;
}