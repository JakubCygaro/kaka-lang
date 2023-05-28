#ifndef LABEL_H
#define LABEL_H

#include <stddef.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

typedef struct {
    char* key;
    size_t value;
} LabelNode;

typedef struct {
    size_t count;
    LabelNode* nodes;
} LabelMap;

LabelMap LabelMap_new();
bool LabelMap_insert(LabelMap*, char*, size_t);
bool LabelMap_get(LabelMap*, char*, size_t*);
void LabelMap_destroy(LabelMap*);
bool LabelMap_contains(LabelMap*, char*);

#endif