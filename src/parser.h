#ifndef PARSER_H
#define PARSER_H
#include <stdio.h>
#include "types.h"
#include <stddef.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "errors.h"
#include <stdio.h>
#include "stringmap.h"

#define PARAM_NONE 0
#define PARAM_1_INT 1
#define PARAM_1_DOUBLE 2
#define PARAM_2_INT 4
#define PARAM_2_DOUBLE 8

static TokenStream* token_stream;
static Instruction* instructions;
static size_t instructions_size;

Instruction* parse_from_file(FILE*, size_t*, StringMap*);

#endif