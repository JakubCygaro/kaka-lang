#ifndef ARGS_PARSE_H
#define ARGS_PARSE_H

typedef struct {
    int preserve;
    char* out_path;
    char* source_path;
} Args;

typedef enum {
    Success = 0,
    UnknownFlag,
    NoSource,
    InvalidFlags,
    TooManyArgs,
} ParseResult;

ParseResult parse_args(char** args, int argn, Args *ret);

#endif