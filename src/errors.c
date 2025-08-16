#include "errors.h"
#include <string.h>


void err_print(const char* fmt, ...){
    va_list args;
    printf("Error: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
void verr_print(const char* fmt, va_list args){
    printf("\nError: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

void crash_on_error(int err){
    int size = 100;
    char buf[size];
#ifdef WIN32
    strerror_s(buf, size, err);
    err_print(buf);
#else
    err_print(strerror(err));
#endif
}
