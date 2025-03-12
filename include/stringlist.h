#ifndef STRINGLIST_H
#define STRINGLIST_H
#include <stddef.h>

typedef struct StringNode {
    /// the actual string to write 
    char* str;
    /// number to append to the string
    size_t num;
    struct StringNode *next;
} StringNode;

typedef struct StringList {
    StringNode *first;
} StringList;

StringList StringList_new();
void StringList_append(StringList *list, StringNode to_add);
void StringList_foreach(const StringList *list, void(*func)(StringNode*, void*), void* data);
void StringList_free(StringList *list);
#endif  
