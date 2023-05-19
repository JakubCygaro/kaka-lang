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

static TokenStream* token_stream;
static Instruction* instructions;
static size_t instructions_size;

Instruction* parse_from_file(FILE*, size_t*);
static Instruction parse_instruction(COMMAND_TYPE);
static void add_instruction(Instruction);
static void ensure_token(TOKEN_TYPE, Token*);
static void ensure_value(VALUE_TYPE, Value*);
static void ensure_value_not(VALUE_TYPE, Value*);
#endif