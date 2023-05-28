#include "types.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        case GREAT:
            printf("great\n");
            break;
        case GREATEQ:
            printf("greateq\n");
            break;
        case LESS:
            printf("less\n");
            break;
        case LESSEQ:
            printf("lesseq\n");
            break;
        case PRINT_STACK:
            printf("_stack\n");
            break;
        case CAST_INT:
            printf("int\n");
            break;
        case CAST_DOUBLE:
            printf("double\n");
            break;
        case LABEL:
            printf("lab\n");
            break;
        case JUMP:
            printf("jmp\n");
            break;
        default:
            printf("unhandled command type\n");
            break;
    }
}

// unsigned long hash(const char *str){
//     unsigned long hash = 5381;
//     int c;
//     while ((c = *str++)){
//         hash = ((hash << 5) + hash) + c;
//     }
//     return hash;
// }

// unsigned long index_for(unsigned long hash){
//     return hash % TABLE_SIZE;
// }

// LabelMap LabelMap_create() {
//     LabelMap m;
//     m.nodes = calloc(TABLE_SIZE, sizeof(LabelNode*));
//     return m;
// }
// void LabelMap_free(LabelMap* map){
//     for (int i = 0; i < TABLE_SIZE; i++){
//         LabelNode* node = map->nodes[i];
//         while(node != NULL){
//             LabelNode next_node = node->
//         }
//     }
// }

// int LabelMap_check(LabelMap* map, LabelNode node){
//     for (size_t i = 0; i < map->size; i++){
//         if(strcmp(map->nodes[i]->key, node.key) == 0)
//             return 1;
//     }
//     return 0;
// }
// int LabelMap_put(LabelMap* self, char* key, size_t val){
//     LabelNode n = {
//         key,
//         val,
//     };
//     if (LabelMap_check(self, n))
//         return 0;
//     self->nodes = realloc(self->nodes, (self->size + 1) * sizeof(LabelNode*));
//     LabelNode* node = calloc(1, sizeof n);
//     *node = n;
//     self->nodes[self->size] = node;
//     return 1;
// }
// LabelNode* LabelMap_get(LabelMap* self, char* key, size_t val){

// }
// void LabelMap_free(LabelMap* map){
//     for (size_t i = 0; i < map->size; i++){
//         free(map->nodes[i]->key);
//         free(map->nodes[i]);
//     }

// }
// LabelMap LabelMap_new(){
//     LabelMap m;
//     m.nodes = malloc(0);
//     m.size = 0;
//     return m;
// }