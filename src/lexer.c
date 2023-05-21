#include "lexer.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdbool.h>

TokenStream* get_token_stream(FILE* source_file){
    TokenStream* t_stream = calloc(1, sizeof(TokenStream));
    t_stream->size = 0;
    t_stream->tokens = calloc(0, sizeof(Token));
    t_stream->position = 0;
    
    size_t line = 1;
    char current_c;
    while((current_c = getc(source_file)) != EOF){

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
        case 'u':
        case 'p':
        case 'r':
        case 's':
        case 't':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'q':
        case 'v':
        case '_':
            Token token = scan_for_instruction(source_file, current_c);
            token.line = line;
            add_token(t_stream, token);
            continue;

        case '\n':
            line++;
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
            add_token(t_stream, token);
        continue;

        case '\"':
            token = scan_for_string(source_file, current_c);

            //Value_print(&token.v);

            token.line = line;
            add_token(t_stream, token);
            continue;
        
        case '#':
            char skipped;
            while ((skipped = getc(source_file)) != EOF) {
                if(skipped == '\n')
                    break;
            }

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
    long start = ftell(source);
    long end = start;
    char c;
    while((c = getc(source)) != EOF){
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
    else
        err_print("unrecognized instruction type");
    return PRINT;
}

static Token scan_for_number(FILE * source, char first){
    ungetc(first, source);
    long start = ftell(source);
    long end = start;
    char c;
    bool minus = false;
    bool dot = false;
    
    while((c = getc(source)) != EOF){
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
    //first is " at this point
    //ungetc(first, source);
    long start = ftell(source);
    long end = start;

    char c;
    while((c = getc(source)) != EOF){
        end++;
        if(c == '\"')
            break;
    }

    fseek(source, start, SEEK_SET);
    long length = end - start - 1;
    char buf[length + 1];
    fread(buf, sizeof(char), length, source);
    getc(source);
    buf[length] = '\0';
    
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