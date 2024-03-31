#include "label.h"

LabelMap LabelMap_new(){
    LabelMap map = {
        0,
        NULL,
    };
    return map;
}
bool LabelMap_contains(LabelMap* self, char* key){
    for(size_t i = 0; i < self->count; i++){
        if(strcmp(key, self->nodes[i].key) == 0)
            return true;
    }
    return false;
}

bool LabelMap_insert(LabelMap* self, char* key, size_t value){
    if(LabelMap_contains(self, key))
        return false;
    self->count++;
    self->nodes = realloc(self->nodes, sizeof(LabelNode) * self->count);
    // char* real_key = calloc(strlen(key), sizeof(char));
    // size_t size =  strlen(key) + 1;
    // strcpy_s(real_key, size, key);
    LabelNode n = {
        key,
        value,
    };

    self->nodes[self->count-1] = n;
    return true;
}

bool LabelMap_get(LabelMap* self, char* key, size_t* ret){
    for(size_t i = 0; i < self->count; i++){
        if(strcmp(key, self->nodes[i].key) == 0){
            if(ret != NULL)
                *ret = self->nodes[i].value;
            return true;
        }
    }
    return false;
}
void LabelMap_destroy(LabelMap* self){
    // for(size_t i = 0; i < self->count; i++){
    //     free(self->nodes[i].key);
    // }
    free(self->nodes);
    self->count = 0;
}