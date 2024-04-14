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
    char *ret = malloc((size + 1) * sizeof(char));
    if(ret == NULL) return NULL;
    strcpy(ret, first);
    strcpy(ret + f_size, second);
    return ret;
}