#include "util.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// appends the second string to the first one, returns a pointer to the allocated product string
char *push_str(char *first, char *second) {
    size_t f_size = strlen(first);
    size_t s_size = strlen(second);
    size_t size = f_size + s_size;
    //printf("f_size: %llu\ns_size: %llu\nsize: %llu\n", f_size, s_size, size);
    char *ret = malloc((size + 1) * sizeof(char));
    if(ret == NULL) return NULL;
    strcpy(ret, first);
    strcpy(ret + f_size, second);
    //ret[size + 1] = '\0';
    //printf("ret: %s\n", ret);
    return ret;
}