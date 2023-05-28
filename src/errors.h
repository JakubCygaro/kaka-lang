#ifndef ERRORS_H
#define ERRORS_H
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdio.h>

#define EXIT_FAILURE 1

void err_print(const char*, ...);
void crash_on_error(errno_t);

#endif