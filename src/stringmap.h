#ifndef STRING_MAP_H
#define STRING_MAP_H


typedef struct StringMap {
    struct StringMapNode *root;

} StringMap;

typedef struct StringMapNode {
    const size_t hash;
    const unsigned char *str;

    struct StringMapNode *left;
    struct StringMapNode *right;
} StringMapNode;

typedef void(*foreach_func_t)(StringMapNode*);

StringMap StringMap_new(void); 
StringMapNode StringMapNode_new(const unsigned char*);
int StringMap_insert(StringMap*, StringMapNode);
void StringMap_free(StringMap*);
void StringMap_foreach(const StringMap*, foreach_func_t);

#endif