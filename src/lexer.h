#ifndef LEXER_H
#define LEXER_H
#include "types.h"
#include <stdio.h>
#include <stddef.h>

TokenStream* get_token_stream(FILE*);

static Token scan_for_instruction(FILE*, char);
static Token scan_for_number(FILE*, char);
static COMMAND_TYPE get_command_type(char*);
static void add_token(TokenStream*, Token);
static Token scan_for_string(FILE*, char);
#endif