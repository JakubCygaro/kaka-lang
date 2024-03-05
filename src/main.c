#include <corecrt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "errors.h"
#include "parser.h"
#include <stdbool.h>
#include "label.h"
#include <vadefs.h>
#include "stringlist.h"

#define MAXSTACK 1000
#define STRINGIFY(A) #A
FILE* source_file;

bool check_ext(const char*, const char*);
void dealloc_values();
void dealloc_inst();
void crash(const char* fmt, ...);
void check_source_ext(char*);
void excecute_instruction(Instruction* inst);
void dealloc_value(Value* v);

#if 1
void emit_instruction(Instruction* inst, FILE *out);
void compile();

StringList stringList;
#endif
//void setup_data(FILE* out);

static Instruction* inst;
static size_t instruction_amount;
static size_t inst_pos = 0;
Value stack[MAXSTACK];
long int pos = 0;
LabelMap label_map;

void print_stack(){
    for(long int i = pos-1; i >= 0; i--)
        Value_print(&stack[i]);
}
void print_full_stack(){
    for(long int i = MAXSTACK-1; i >= 0; i--)
        Value_print(&stack[i]);
}

void push_value(Value v){
    if (pos >= MAXSTACK){

        crash("stack overflowed");
    }
    stack[pos++] = v;
}
Value pop_value(){
    if (pos < 0){
        //CommandType_print(inst[inst_pos].c_type);
        crash("empty stack");
    }
    return stack[--pos];
}


void setup_labels(){
    label_map = LabelMap_new();
    while(inst_pos < instruction_amount){
        switch (inst[inst_pos].c_type) {
            case LABEL:
                bool out = LabelMap_insert(&label_map, inst[inst_pos].v.string, inst_pos);
                if(!out)
                    crash("redeclaration of label `%s`", inst[inst_pos].v.string);
            
            default:
            ;
        }
        inst_pos++;
    }

    inst_pos = 0;
    while(inst_pos < instruction_amount){
        switch (inst[inst_pos].c_type) {
            case JUMP:
                size_t ret;
                bool out = LabelMap_get(&label_map, inst[inst_pos].v.string, &ret);
                if(!out)
                    crash("no such label `%s`", inst[inst_pos].v.string);
                inst[inst_pos].jump_dest = ret;
            default:
            ;
        }
        inst_pos++;
    }
    inst_pos = 0;
}


void dealloc_values(){
    for (int i = 0; i < MAXSTACK; i++){
        dealloc_value(&stack[i]);
    }
}
/// deallocates a value if it contains a pointer to allocated memory
void dealloc_value(Value* v){
    if(v->v_type != STRING)
            return;
    free(v->string);
}
/// deallocates all values that instructions might contain (char* )
void dealloc_inst(){
    for(size_t i = 0; i < instructions_size; i++)
        dealloc_value(&instructions[i].v);
}



int main(int argc, char** args){

    if (argc != 2)
        err_print("source file path was not provided.");


    check_source_ext(args[1]);


    errno_t error = fopen_s(&source_file, args[1], "r");
    if (error != 0){
        crash_on_error(error);
    }
    inst = parse_from_file(source_file, &instruction_amount);
    fclose(source_file);

    setup_labels();

#ifndef COMPILER
    while(inst_pos < instruction_amount){
        excecute_instruction(&inst[inst_pos]);

        inst_pos++;
    }
#else
    stringList = StringList_new();
    compile();
#endif
    dealloc_values();
    dealloc_inst();
#ifdef COMPILER
    StringList_free(&stringList);
#endif
    free(inst);
    return 0;
}
void excecute_instruction(Instruction* inst){
    switch (inst->c_type) {
        case PRINT_STACK:
            print_stack();
            break;
        case PUSH:
            push_value(inst->v);
            break;
        case POP:
            Value v = pop_value();
            dealloc_value(&v);
            break;
        case PRINT:
            v = pop_value();
            Value_print(&v);
            push_value(v);
            break;
        case CAST_INT:
            v = pop_value();
            Value res;
            res.v_type = INT;
            if(v.v_type == DOUBLE){
                res.i = (int)v.d;
            }
            else if(v.v_type == INT){
                res.i = v.i;
                push_value(res);
            }
            else {
                crash("invalid cast exception, line: %lld", inst_pos + 1);
            }
            push_value(res);
            break;
        case CAST_DOUBLE:
            v = pop_value();
            res.v_type = DOUBLE;
            if(v.v_type == INT){
                res.d = (double)v.i;
            }
            else if(v.v_type == DOUBLE){
                res.d = v.d;
                push_value(res);
            }
            else {
                crash("invalid cast exception, line: %lld", inst_pos + 1);
            }
            push_value(res);
            break;
        case LABEL:
            break;

        case JUMP:
            inst_pos = inst->jump_dest - 1;
            break;
        case ADD:
            Value a = pop_value();
            Value b = pop_value();
            if(a.v_type == INT && b.v_type == INT){
                res.v_type = INT;
                res.i = a.i + b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = a.i + b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.v_type = DOUBLE;
                res.d = a.d + b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = a.d + b.d;
            }
            else{
                crash("invalid addition operands");
            }

            push_value(res);
            break;
        case SUB:
            a = pop_value();
            b = pop_value();
            if(a.v_type == INT && b.v_type == INT){
                res.v_type = INT;
                res.i = b.i - a.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = b.i - a.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.v_type = DOUBLE;
                res.d = b.d - a.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = b.d - a.d;
            }
            else 
                crash("invalid subtraction operands");

            push_value(res);
            break;
        case MUL:
            a = pop_value();
            b = pop_value();
            if(a.v_type == INT && b.v_type == INT){
                res.v_type = INT;
                res.i = a.i * b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = (double)a.i * b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.v_type = DOUBLE;
                res.d = (double)a.d * b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = a.d * b.d;
            }
            else 
                crash("invalid multiplication operands");

            push_value(res);
            break;
        case DIV:
            Value divisor = pop_value();
            a = pop_value();
            if(divisor.v_type == INT)
                if(divisor.i == 0)
                    crash("zero divisor");
            if(divisor.v_type == DOUBLE)
                if(divisor.d == 0)
                    crash("zero divisor");

            if(a.v_type == INT && divisor.v_type == INT){
                res.v_type = DOUBLE;
                res.d = (double)a.i / (double)divisor.i;
            }
            else if (a.v_type == INT && divisor.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = (double)a.i / divisor.d;
            }
            else if (a.v_type == DOUBLE && divisor.v_type == INT){
                res.v_type = DOUBLE;
                res.d = a.d / (double)divisor.i;
            }
            else if (a.v_type == DOUBLE && divisor.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = a.d / divisor.d;
            }
            else 
                crash("invalid division operands");

            push_value(res);
            break;
        case MOD:
            divisor = pop_value();
            a = pop_value();
            if(divisor.v_type == INT)
                if(divisor.i == 0)
                    crash("zero divisor");
            if(divisor.v_type == DOUBLE)
                if(divisor.d == 0)
                    crash("zero divisor");
            
            if(a.v_type == INT && divisor.v_type == INT){
                res.v_type = INT;
                res.i = a.i % divisor.i;
            }
            else 
                crash("invalid modulo operands");
            
            push_value(res);
            break;
        case CMP:
            a = pop_value();
            b = pop_value();
            res.v_type = INT;
            if(a.v_type == INT && b.v_type == INT){
                res.i = a.i == b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.i = (double)a.i == b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.i = a.d == (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.i = a.d == b.d;
            }
            else if (a.v_type == STRING && b.v_type == STRING){
                res.i = strcmp(a.string, b.string) == 0;
                dealloc_value(&a);
                dealloc_value(&b);
            }
            else 
                crash("invalid comparison operands");
            
            push_value(res);
            break;
        case GREAT:
            b = pop_value();
            a = pop_value();
            res.v_type = INT;
            if(a.v_type == INT && b.v_type == INT){
                res.i = a.i > b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.i = (double)a.i > b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.i = a.d > (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.i = a.d > b.d;
            }
            else if (a.v_type == STRING && b.v_type == STRING){
                res.i = strcmp(a.string, b.string) > 0;
                dealloc_value(&a);
                dealloc_value(&b);
            }
            else 
                crash("invalid great operands");
            
            push_value(res);
            break;
        case GREATEQ:
            b = pop_value();
            a = pop_value();
            res.v_type = INT;
            if(a.v_type == INT && b.v_type == INT){
                res.i = a.i >= b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.i = (double)a.i >= b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.i = a.d >= (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.i = a.d >= b.d;
            }
            else if (a.v_type == STRING && b.v_type == STRING){
                res.i = strcmp(a.string, b.string) >= 0;
                dealloc_value(&a);
                dealloc_value(&b);
            }
            else 
                crash("invalid greateq operands");
            
            push_value(res);
            break;
        case LESS:
            b = pop_value();
            a = pop_value();
            res.v_type = INT;
            if(a.v_type == INT && b.v_type == INT){
                res.i = a.i < b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.i = (double)a.i < b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.i = a.d < (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.i = a.d < b.d;
            }
            else if (a.v_type == STRING && b.v_type == STRING){
                res.i = strcmp(a.string, b.string) < 0;
                dealloc_value(&a);
                dealloc_value(&b);
            }
            else 
                crash("invalid less operands");
            
            push_value(res);
            break;
        case LESSEQ:
            b = pop_value();
            a = pop_value();
            res.v_type = INT;
            if(a.v_type == INT && b.v_type == INT){
                res.i = a.i <= b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.i = (double)a.i <= b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.i = a.d <= (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.i = a.d <= b.d;
            }
            else if (a.v_type == STRING && b.v_type == STRING){
                res.i = strcmp(a.string, b.string) <= 0;
                dealloc_value(&a);
                dealloc_value(&b);
            }
            else 
                crash("invalid lesseq operands");
            
            push_value(res);
            break;
        case IF:
            v = pop_value();
            bool bo;
            if(v.v_type == INT){
                bo = v.i == 1;
            }
            else if (v.v_type == DOUBLE){
                bo = v.d == 1;
            }
            else {
                dealloc_value(&v);
                crash("invalid if statement argument");
            }
            if(!bo){
                inst_pos++;
            }
            break;
        case CLONE:
            a = pop_value();
            push_value(a);
            push_value(a);            
            break;
        case AND:
            a = pop_value();
            b = pop_value();
            res.v_type = INT;
            if(a.v_type == INT && b.v_type == INT){
                res.i = a.i && b.i;
            }
            else {
                crash("invalid logical and aruments");
            }
            push_value(res);
            break;
        case OR:
            a = pop_value();
            b = pop_value();
            res.v_type = INT;
            if(a.v_type == INT && b.v_type == INT){
                res.i = a.i || b.i;
            }
            else {
                crash("invalid logical or aruments");
            }
            push_value(res);
            break;
        case NOT:
            a = pop_value();
            res.v_type = INT;
            if(a.v_type == INT){
                res.i = !a.i;
            }
            else {
                crash("invalid logical negation arument");
            }
            push_value(res);
            break;
    }
}

void check_source_ext(char* path){
    if (!check_ext(path, ".kaka"))
        err_print("the specified file is not a .kaka file");
}

void crash(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    fclose(source_file);
    dealloc_values();
    dealloc_inst();
    free(inst);
    verr_print(fmt, args);
}

bool check_ext(const char* path, const char* match){
    size_t match_l = strlen(match);
    size_t path_l = strlen(path);

    if(match_l >= path_l)
        return false;
    
    size_t start = path_l - match_l;

    return strcmp(path + start, match) == 0;
}

void write_string_data(StringNode *node, void *data){
    FILE* out = data;
    // fprintf(out, 
    //     "str_%lld db '%s', 0, 10\n"
    //     "str_%lld.len = $ - str_%lld\n", node->num, node->str, node->num, node->num);
    // fprintf(out, 
    //     "str_%lld db '%s', 10, 0\n", node->num, node->str);
    fprintf(out, 
        "str_%lld db ", node->num);
    int c;
    int i = 0;
    while(true){
        c = node->str[i++];
        if(c == '\0'){
            fprintf(out, "%x\n", c);
            break;
        } else {
            fprintf(out, "%x,", c);
        }

    }


    fputc('\n', out);
}

void compile(){
    FILE* output;
    fopen_s(&output, "compilation.asm", "w+");
    if (output == NULL){
        crash("could not create intermediate .asm file");
    }

    //format, macros
    fprintf(output, 
        "format PE64 console\n"
        "entry start\n"
        "macro import [libname] {\n"
        "forward\n"
        "   dd 0, 0, 0, RVA NAME__#libname, RVA TABLE__#libname\n"
        "common\n"
        "   dd 0, 0, 0, 0, 0\n"
        "forward\n"
        "   NAME__#libname db `libname, \".DLL\", 0\n"
        "}\n"
        "macro functions libname, [funcname] {\n"
        "common\n"
        "   TABLE__#libname:\n"
        "forward\n"
        "   funcname dq RVA _#funcname\n"
        "common\n"
        "   dq 0\n"
        "forward\n"
        "   _#funcname db 0, 0, `funcname, 0\n"
        "}\n");
    
    //import section
    fprintf(output, 
        "section '.idata' import data readable writable\n"
        "import KERNEL32, msvcrt\n"
        "functions KERNEL32, ExitProcess, GetStdHandle, WriteFile\n"
        "functions msvcrt, printf\n");

    //code section
    fprintf(output, 
        "section '.text' code readable executable\n"
        "start:\n"
        "   sub rsp, 8 ; align stack\n"/*
        "   sub rsp, 4 * 8 ; reserve stack for call\n"
        "   mov rcx, -11 ; stdout handle\n"
        "   call [GetStdHandle]\n"
        "   mov [std_out], rax ; save handle\n"
        "   add rsp, 4 * 8 ; clean stack\n"*/);
    
    while(inst_pos < instruction_amount){
        //printf(STRINGIFY(instruction_amount) " : %lld\n", instruction_amount);
        emit_instruction(&inst[inst_pos], output);
        inst_pos++;
    }

    fprintf(output, 
        "   sub rsp, 4 * 8 ; reserve stack for call\n"
        "   mov rcx, 0\n"
        "   call [ExitProcess] ; exit \n");

    //data section
    fprintf(output, 
        "section '.data' data readable writable\n"
        //"std_out dq 0 ; STDOUT_HANDLE\n"
        "fmt_integer db '%%d', 10, 0 ; fmt string for integer print\n"
        "; STRING DATA SECTION\n");
    //string data section
    StringList_foreach(&stringList, write_string_data, (void*)output);


    fclose(output);
    system("fasm ./compilation.asm");
}


static bool STRING_TOP = false;

void emit_instruction(Instruction* inst, FILE *out){
    //printf("*inst: %p\n", inst);
    switch (inst->c_type) {
        case PUSH: {
            if(inst->v.v_type == DOUBLE || inst->v.v_type == NONE)
                crash("compile error: value not supported");
            if(inst->v.v_type == STRING){
                char *str = inst->v.string;
                StringNode node = {
                    .next = NULL,
                    .num = inst_pos,
                    .str = str,
                };
                StringList_append(&stringList, node);

                fprintf(out, 
                    "   push str_%lld ; push str\n", node.num);
                STRING_TOP = true;
            } else {
                fprintf(out, 
                    "   push %d ; push integer\n", inst->v.i);
                STRING_TOP = false;
            }
        }break;
        case POP: {
            fprintf(out, 
                "   add rsp, 8 ; pop\n");
        }break;
        case ADD: {
            fprintf(out, 
                "   pop rax\n"
                "   pop rcx\n"
                "   add rax, rcx\n"
                "   push rax ; add\n");
        }break;
        case SUB: {
            fprintf(out, 
                "   pop rax\n"
                "   pop rcx\n"
                "   sub rax, rcx\n"
                "   push rax ; sub\n");
        }break;
        case MUL: {
            fprintf(out, 
                "   pop rax\n"
                "   pop rcx\n"
                "   imul rax, rcx\n"
                "   push rax ; mul\n");
        }break;
        case PRINT: {
            if(!STRING_TOP){
                //crash("compile error: only string printing supported");
                fprintf(out,
                    "   mov rcx, fmt_integer ; integer print\n"
                    "   pop rdx\n"
                    "   sub rsp, 4 * 8\n"
                    "   call [printf]\n"
                    "   add rsp, 4 * 8\n");
            } else {
                fprintf(out, 
                    "   pop rcx ; load string\n"
                    "   sub rsp, 4*8 ; reserve stack\n"
                    "   call [printf]\n"
                    "   add rsp, 4 * 8 ; stack cleanup\n");
            }
        }break;
        default:
            crash("compile error: instruction not supported");
    }
}