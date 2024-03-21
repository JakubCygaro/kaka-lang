#include "lexer.h"
#include "errors.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t line;
static size_t column;

TokenStream* get_token_stream(FILE* source_file){
    TokenStream* t_stream = calloc(1, sizeof(TokenStream));
    t_stream->size = 0;
    t_stream->tokens = calloc(0, sizeof(Token));
    t_stream->position = 0;
    
    line = 1;
    column = 0;
    
    char current_c;
    while((current_c = getc(source_file)) != EOF){
        column++;
        //printf("%c\n", current_c);

        switch (current_c) {
        
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'w':
        case 'v':
        case 'x':
        case 'y':
        case 'z':
        case 'q':
        case '_':
            Token token = scan_for_instruction(source_file, current_c);
            token.line = line;
            token.col = column;
            add_token(t_stream, token);
            continue;

        case '\n':
            line++;
            column = 0;
        case ' ':
        case '\t':
            continue;

        case '-':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
            token = scan_for_number(source_file, current_c);
            token.line = line;
            token.col = column;
            add_token(t_stream, token);
        continue;

        case '\"':
            token = scan_for_string(source_file, current_c);
            token.line = line;
            token.col = column;
            add_token(t_stream, token);
            continue;
        
        case '#':
            char skipped;
            while ((skipped = getc(source_file)) != EOF) {
                if(skipped == '\n')
                    break;
            }
            line++;
            column = 0;
            continue;

        case ';':
            Token empty = {
                .t_type = EMPTY,
            };
            empty.v.v_type = NONE;
            empty.v.none = NULL;
            empty.line = line;
            empty.col = column;
            add_token(t_stream, empty);
            continue;

        default:
            char buf[60];
            sprintf(buf, "unrecognized character `%c` line: %lld", 
                current_c, line);
            err_print(buf);
        }
    }
    // printf("size: %llu\n pos: %llu\n", t_stream.size, t_stream.position);

    return t_stream;
}
static Token scan_for_instruction(FILE* source, char first){
    ungetc(first, source);
    column--;
    long start = ftell(source);
    long end = start;
    char c;
    while((c = getc(source)) != EOF){
        column++;
        if(c == ' ' || c == '\n' || c == '\t')
            break;
        else if (isdigit(c))
            err_print("instruction name cannot contain a number");
        else {
            end++;
        }
    }
    fseek(source, start, SEEK_SET);
    long length = end - start;
    char buf[length + 1];
    fread(buf, sizeof(char), length, source);
    buf[length] = '\0';
    //printf("%s\n", buf);
    COMMAND_TYPE c_type = get_command_type(buf);
    Token t;
    t.t_type = COMMAND;
    t.c_type = c_type;
    return t;
}



static COMMAND_TYPE get_command_type(char * cmd_str) {
    if(strcmp(cmd_str, "push") == 0)
        return PUSH;
    else if (strcmp(cmd_str, "pop") == 0)
        return POP;
    else if (strcmp(cmd_str, "add") == 0)
        return ADD;
    else if (strcmp(cmd_str, "sub") == 0)
        return SUB;
    else if (strcmp(cmd_str, "print") == 0)
        return PRINT;
    else if (strcmp(cmd_str, "div") == 0)
        return DIV;
    else if (strcmp(cmd_str, "mul") == 0)
        return MUL;
    else if (strcmp(cmd_str, "mod") == 0)
        return MOD;
    else if (strcmp(cmd_str, "cmp") == 0)
        return CMP;
    else if (strcmp(cmd_str, "great") == 0)
        return GREAT;
    else if (strcmp(cmd_str, "greateq") == 0)
        return GREATEQ;
    else if (strcmp(cmd_str, "less") == 0)
        return LESS;
    else if (strcmp(cmd_str, "lesseq") == 0)
        return LESSEQ;
    else if (strcmp(cmd_str, "_stack") == 0)
        return PRINT_STACK;
    else if (strcmp(cmd_str, "int") == 0)
        return CAST_INT;
    else if (strcmp(cmd_str, "double") == 0)
        return CAST_DOUBLE;
    else if (strcmp(cmd_str, "lab") == 0)
        return LABEL;
    else if (strcmp(cmd_str, "jmp") == 0)
        return JUMP;
    else if (strcmp(cmd_str, "if") == 0)
        return IF;
    else if (strcmp(cmd_str, "clone") == 0)
        return CLONE;
    else if (strcmp(cmd_str, "and") == 0)
        return AND;
    else if (strcmp(cmd_str, "or") == 0)
        return OR;
    else if (strcmp(cmd_str, "not") == 0)
        return NOT;
    else if (strcmp(cmd_str, "assert") == 0)
        return ASSERT;
    else if (strcmp(cmd_str, "swap") == 0)
        return SWAP;
    else
        err_print("unrecognized instruction type `%s` (%lld:%lld)\n", cmd_str, line, column);
    return PRINT;
}

static Token scan_for_number(FILE * source, char first){
    ungetc(first, source);
    column--;
    long start = ftell(source);
    long end = start;
    char c;
    bool minus = false;
    bool dot = false;
    
    while((c = getc(source)) != EOF){
        column++;
        if(c == ' ' || c == '\n' || c == '\t')
            break;
        else if (c == '-' && !minus){
            end++;
            minus = true;
        }
        else if (c == '.' && !dot){
            end++;
            dot = true;
        }
        else if (!isdigit(c))
            err_print("number literal contains an invalid character");
        else {
            end++;
        }
    }
    fseek(source, start, SEEK_SET);
    long length = end - start;
    char buf[length + 1];
    fread(buf, sizeof(char), length, source);
    buf[length] = '\0';
    if (!dot) {
        int i_value;
        sscanf_s(buf, "%d", &i_value);
        Token t;
        t.t_type = VALUE;
        Value v;
        v.v_type = INT;
        v.i = i_value;
        t.v = v;
        return t;
    }
    else {
        double d_value;
        sscanf_s(buf, "%lf", &d_value);
        Token t;
        t.t_type = VALUE;
        Value v;
        v.v_type = DOUBLE;
        v.d = d_value;
        t.v = v;
        return t;
    }
}
static Token scan_for_string(FILE* source, char first){
    //the first (") is at this point
    long start = ftell(source);
    long end = start;

    char c;
    while((c = getc(source))){
        column++;
        if(c == '\n')
            fprintf(stderr, "Error: no multiline string support (%lld:%lld)\n", 
                line, column);
        if (c == '\\') {
            scan_special_char(source);
        } else if (c == '\"'){
            break;
        }
        end++;
        if(c == EOF) {
            fprintf(stderr, "Error: missing parentheses (%lld:%lld)\n", 
                line, column);
            exit(1);
        }
    }

    fseek(source, start, SEEK_SET);
    long length = end - start;// - 1;
    char buf[length + 1];
    
    for(size_t i = 0; i < length; i++){
        c = getc(source);
        if(c == '\\')
            c = scan_special_char(source);
        buf[i] = c;
    }
    
    //fread(buf, sizeof(char), length, source);
    getc(source); // skip the final (")

    buf[length] = '\0';
    //printf("buf: %s\n", buf);
    char* string = malloc((length + 1) * sizeof(char));
    strcpy_s(string, (length + 1) * sizeof(char), buf);
    Token t;
    t.t_type = VALUE;
    Value v;
    v.v_type = STRING;
    v.string = string;
    t.v = v;
    return t;

}
static void add_token(TokenStream* t_stream, Token token){
    t_stream->size++;
    t_stream->tokens = realloc(t_stream->tokens, t_stream->size * sizeof(Token));
    t_stream->tokens[t_stream->size-1] = token;
}

#define SPECIALC(C) case C : return 

static unsigned char scan_special_char(FILE* source){
    int c;
    column++;
    switch ((c = getc(source))) {
        case 'n'    : return '\n';
        case 'r'    : return '\r';
        case 'b'    : return '\b';
        case '0'    : return '\0';
        case 'f'    : return '\f';
        case 't'    : return '\t';
        case 'a'    : return '\a';
        case 'v'    : return '\v';
        case '\\'   : return '\\';
        case '?'    : return '\?';
        case 'e'    : return '\e';
        case '\''   : return '\'';
        case '"'    : return '"';
        default: fprintf(stderr, "unknown escape sequence '\\%c' (%lld:%lld)", c, line, column); exit(1);
    }
}