#include "types.h"
#include <stdio.h>
#include <stdlib.h>

void TokenStream_free(TokenStream* self){
    free(self->tokens);
    free(self);
}
Token* TokenStream_gett(TokenStream* self){
    if(self->position >= self->size)
        return NULL;
    Token* ret =  &self->tokens[self->position++];    
    return ret;
}
void TokenStream_ungett(TokenStream* self){
    self->position--;
}

void Value_print(Value* self){
    switch (self->v_type) {
        case INT:
            printf("%d\n", self->i);
            break;
        case DOUBLE:
            printf("%lf\n", self->d);
            break;
        case STRING:
            printf("%s\n", self->string);
            break;
        default:
            printf("NULL");
            break;
    }
}
void Token_print(Token* self){
    switch (self->t_type) {
        case COMMAND:
            printf("Command: ");
            CommandType_print(self->c_type);
            break;
        case VALUE:
            printf("Value: ");
            Value_print(&self->v);
            break;
    }
}
void CommandType_print(COMMAND_TYPE c_type){
    switch (c_type) {
        case PRINT:
            printf("print\n");
            break;
        case PUSH:
            printf("push\n");
            break;
        case POP:
            printf("pop\n");
            break;
        case ADD:
            printf("add\n");
            break;
        case SUB:
            printf("sub\n");
            break;
        case DIV:
            printf("div\n");
            break;
        case MUL:
            printf("mul\n");
            break;
        case MOD:
            printf("mod\n");
            break;    
        case CMP:
            printf("cmp\n");
            break;
        default:
            printf("unhandled command type\n");
            break;
    }
}