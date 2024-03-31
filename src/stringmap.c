#include "stringmap.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

static size_t hash(const unsigned char *str) {
    size_t hash = 5381;
    int c;
    while((c = *str++))
        hash = ((hash << 5) +  hash) + c;
    return hash;
}

static int insert_impl(StringMapNode* parent, StringMapNode *node){
    if(node->hash > parent->hash && parent->left == NULL){
        parent->left = node;
        return 1;
    } 
    else if(node->hash > parent->hash && parent->left != NULL){
        return insert_impl(parent->left, node);
    }
    else if(node->hash < parent->hash && parent->right == NULL) {
        parent->right = node;
        return 1;
    }
    else if(node->hash < parent->hash && parent->right != NULL) {
        return insert_impl(parent->right, node);
    }
    return 0;
}

static void foreach_impl(StringMapNode *parent, foreach_func_t func, void* data) {
    func(parent, data);
    if(parent->left != NULL){
        foreach_impl(parent->left, func, data);
    }
    if (parent->right != NULL){
        foreach_impl(parent->right, func, data);
    }
}

StringMap StringMap_new() {
    StringMap map = {
        .root = NULL
    };
    return map;
}

StringMapNode StringMapNode_new(const unsigned char *str) {
    size_t h = hash(str);
    StringMapNode node = {
        .hash = h,
        .str = str,
        .left = NULL,
        .right = NULL,
    };
    return node;
}

int StringMap_insert(StringMap* self, StringMapNode node) {
    StringMapNode *node_p = malloc(sizeof node);
    if (node_p == NULL){
        fprintf(stderr, "could not allocate memory for string map node\n");
        exit(1);
    }
    memcpy(node_p, &node, sizeof node);
    if (self->root == NULL){
        self->root = node_p;
        return 1;
    }
    return insert_impl(self->root, node_p);
}

void StringMap_foreach(const StringMap *self, foreach_func_t func, void* data){
    if(self->root == NULL) return;
    foreach_impl(self->root, func, data);
}

void free_impl(StringMapNode *parent) {
    if(parent->left != NULL){
        free_impl(parent->left);
    }
    if (parent->right != NULL){
        free_impl(parent->right);
    }
    free(parent);
}

void StringMap_free(StringMap* self) {
    if(self->root == NULL) return;
    free_impl(self->root);
}

static StringMapNode *get_impl(StringMapNode *parent, size_t hash){
    if(parent->hash == hash) return parent;

    if(hash > parent->hash){
        if(parent->left == NULL) return NULL;
        return get_impl(parent->left, hash);
    }
    else if (hash < parent->hash){
        if(parent->right == NULL) return NULL;
        return get_impl(parent->right, hash);
    }
    return NULL;
}

StringMapNode *StringMap_get(StringMap *self, const unsigned char* str) {
    size_t h = hash(str);
    if(self->root == NULL) return NULL;
    return get_impl(self->root, h);
}