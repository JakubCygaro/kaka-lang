#include "errors.h"


void err_print(const char* fmt, ...){
    va_list args;
    printf("Error: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
void verr_print(const char* fmt, va_list args){
    printf("Error: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void crash_on_error(errno_t err){
    int size = 100;
    char buf[size];
    strerror_s(buf, size, err);
    err_print(buf);
}