#include "stringlist.h"
#include <stdio.h>
#include <stdlib.h>

void StringList_append(StringList *list, StringNode to_add){
    if(list->first == NULL){
        list->first = malloc(sizeof to_add);
        if(list->first == NULL){
            fprintf(stderr, "get more RAM");
            exit(1);
        }
        list->first->num = to_add.num;
        list->first->str = to_add.str;
        list->first->next = NULL;
        return;
    } 
    StringNode *node = list->first;
    while(node->next != NULL){
        node = node->next;
    }
    StringNode *new = malloc(sizeof(StringNode));
    if(new == NULL){
        fprintf(stderr, "get more RAM");
        exit(1);
    }
    new->num = to_add.num;
    new->str = to_add.str;
    new->next = NULL;
    node->next = new;
}
StringList StringList_new(){
    StringList list = {
        .first = NULL
    };
    return list;
}
void StringList_foreach(const StringList *list, void(*func)(StringNode*, void*), void* data){
    StringNode *node = list->first;

    if(node == NULL) return;
    func(node, data);
    while(node->next != NULL){
        node = node->next;
        func(node, data);
    }
}
void StringList_free(StringList *list) {
    StringNode *node = list->first;
    while(node != NULL){
        StringNode *tmp = node->next;
        free(node);
        node = tmp;
    }
}