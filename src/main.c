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

#define MAXSTACK 10000
#define STRINGIFY(A) #A
FILE* source_file;
#define println(FMT, ...) printf(FMT "\n", ...)

bool check_ext(const char*, const char*);
void dealloc_values();
void dealloc_inst();
void crash(const char* fmt, ...);
void check_source_ext(char*);
void excecute_instruction(Instruction* inst);
void dealloc_value(Value* v);
void stack_printf(const char* fmt, Value values[], size_t val_amout);
#if 1
void emit_instruction(Instruction* inst, FILE *out);
void compile();
void emit_print(int, FILE*);
StringList stringList;
static bool compile_assert = false;
#endif
//void setup_data(FILE* out);

static Instruction* inst_arr;
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
    if (pos - 1 < 0){
        crash("empty stack");
    }
    return stack[--pos];
}


void setup_labels(){
    label_map = LabelMap_new();
    while(inst_pos < instruction_amount){
        switch (inst_arr[inst_pos].c_type) {
            case LABEL:
                bool out = LabelMap_insert(&label_map, inst_arr[inst_pos].v.string, inst_pos);
                if(!out)
                    crash("redeclaration of label `%s`", inst_arr[inst_pos].v.string);
            
            default:
            ;
        }
        inst_pos++;
    }

    inst_pos = 0;
    while(inst_pos < instruction_amount){
        switch (inst_arr[inst_pos].c_type) {
            case JUMP:
                size_t ret;
                bool out = LabelMap_get(&label_map, inst_arr[inst_pos].v.string, &ret);
                if(!out)
                    crash("no such label `%s`", inst_arr[inst_pos].v.string);
                inst_arr[inst_pos].jump_dest = ret;
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
    inst_arr = parse_from_file(source_file, &instruction_amount);
    fclose(source_file);

    setup_labels();

#ifndef COMPILER
    while(inst_pos < instruction_amount){
        excecute_instruction(&inst_arr[inst_pos]);

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
    free(inst_arr);
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
        case PRINT: {
                v = pop_value();
                if(v.v_type != STRING){
                    crash("format string not provided for print instruction");
                }
                char* fmt = v.string;
                int argn = inst->v.i;
                Value args[argn];
                for (int i = 0; i < argn; i++){
                    args[i] = pop_value();
                }
                stack_printf(fmt, args, argn);
                for (int i = argn - 1; i >= 0; i--){
                    push_value(args[i]);
                }
            } break;
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
                res.i = a.i - b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = (double)a.i - b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.v_type = DOUBLE;
                res.d = a.d - (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = a.d - b.d;
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
            a = pop_value();
            divisor = pop_value();
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
                crash("invalid modulo operand types (must be two integers)");
            
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
            a = pop_value();
            b = pop_value();
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
            a = pop_value();
            b = pop_value();
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
            a = pop_value();
            b = pop_value();
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
            a = pop_value();
            b = pop_value();
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
                crash("invalid logical and aruments (%lld:%lld)", inst->line, inst->col);
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
        case ASSERT: {
            a = pop_value();
            if(a.v_type != INT){
                crash("Assertion failed: value not of type int (%lld:%lld)", inst->line, inst->col);
            }
            if(!a.i){
                crash("Assertion failed: \"%s\" (%lld:%lld)", inst->v.string, inst->line, inst->col);
            }
        }break;
        case SWAP: {
            a = pop_value();
            b = pop_value();
            push_value(a);
            push_value(b);
        }
    }
}

void ensure_value_print(VALUE_TYPE type, Value *val){
    if(val->v_type != type)
        crash("wrong argument provided for format string");
}

void stack_printf(const char* fmt, Value values[], size_t val_amout){
    size_t i = 0;
    size_t val_i = 0;
    char c;
    bool special = false;
    while((c = fmt[i++]) != '\0'){
        if(c != '%' && !special) {
            fputc(c, stdout);
            continue;
        } else if (!special) {
            special = true;
            continue;
        }
        if(val_i >= val_amout) {
            crash("too few arguments provided for format string");
        }
        switch (c) {
            case 's':{
                ensure_value_print(STRING, &values[val_i]);
                fprintf(stdout, "%s", values[val_i++].string);
            }break;
            case 'd':{
                ensure_value_print(INT, &values[val_i]);
                fprintf(stdout, "%d", values[val_i++].i);
            }break;
            case 'f': {
                ensure_value_print(DOUBLE, &values[val_i]);
                fprintf(stdout, "%lf", values[val_i++].d);
            }break;

            default: {
                crash("unrecognized format in string");
            }
        }
        special = false;
    }
    if(special) crash("premature end of format string");
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
    free(inst_arr);
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
    fprintf(out, 
        "str_%lld db ", node->num);
    int c;
    int i = 0;
    while(true){
        c = node->str[i++];
        if(c == '\0'){
            fprintf(out, "0x%x\n", c);
            break;
        } else {
            fprintf(out, "0x%x,", c);
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
        "entry _start\n"
        "macro import [libname] {\n"
        "forward\n"
        "   dd 0, 0, 0, RVA NAME__#libname, RVA TABLE__#libname\n"
        "common\n"
        "   dd 0, 0, 0, 0, 0\n"
        "forward\n"
        "   NAME__#libname db `libname, \".DLL\", 0\n"
        "}\n"
        "macro normalize reg* { ; make a value in a register either one or zero\n"
        "common\n"
        "   cmp reg, 0  ; does the value in the register equal 0?\n"
        "   mov reg, 0 ; if it is not equal to zero, 1 will remain there\n"
        "   je @f\n"
        "   mov reg, 1 ; if it was zero move 0 there\n"
        "   @@:\n"
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
        "_start:\n"
        "   sub rsp, 8 ; align stack\n");

    while(inst_pos < instruction_amount){
        emit_instruction(&inst_arr[inst_pos], output);
        inst_pos++;
    }

    //process exit 
    fprintf(output, 
        "   sub rsp, 4 * 8 ; reserve stack for call\n"
        "   mov rcx, 0\n"
        "   call [ExitProcess] ; exit \n");
    
    if(compile_assert)
        fprintf(output, 
            "_assert: ; assert procedure\n"
            "   mov rax, rsp\n"
            "   mov rcx, [rax + 16] ; load int arg\n"
            "   cmp rcx, 0 ; check if false\n"
            "   jne .exit ; exit if not false\n"
            "   mov rcx, [rax + 8] ; load string arg\n"
            "   sub rsp, 4 * 8; reserve stack\n"
            "   call [printf] ; print assert failure message\n"
            "   mov rcx, 0\n"
            "   call [ExitProcess]\n"
            ".exit:\n"
            "   ret ; return\n");

    //data section
    fprintf(output, 
        "section '.data' data readable writable\n"
        //"fmt_integer db '%%d', 10, 0 ; fmt string for integer print\n"
        "; STRING DATA SECTION\n");
    //string data section
    StringList_foreach(&stringList, write_string_data, (void*)output);


    fclose(output);
    system("fasm ./compilation.asm");
}


static bool STRING_TOP = false;

void emit_instruction(Instruction* inst, FILE *out){
    switch (inst->c_type) {
        case PUSH: {
            if(inst->v.v_type == NONE)
                crash("value not supported");
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
            } else if (inst->v.v_type == DOUBLE) {
                fprintf(out, 
                    "   mov rax, %lf\n"
                    "   push rax ; push double\n", inst->v.d);
            }else {
                fprintf(out, 
                    "   push %d ; push integer\n", inst->v.i);
            }
        }break;
        case POP: {
            fprintf(out, 
                "   add rsp, 8 ; pop\n");
        }break;
        case ADD: 
            int params = inst->v.i;
        {
            if(params == PARAM_1_INT){
                fprintf(out, 
                    "   pop rax\n"
                    "   pop rcx\n"
                    "   add rax, rcx\n"
                    "   push rax ; add\n");
            } else if(params == PARAM_1_DOUBLE){
                fprintf(out, 
                    "   fld qword [rsp]; load first double from stack\n"
                    "   add rsp, 8\n"
                    "   fld qword [rsp] ; load second double from stack\n"
                    "   faddp ; add st0 to st1 and pop the FPU stack\n"
                    "   fstp qword [rsp] ; add doubles\n");
            } else if (params == (PARAM_1_INT | PARAM_2_DOUBLE)){
                fprintf(out, 
                    "   fild qword [rsp] ; load signed integer\n"
                    "   add rsp, 8 ; pop from stack\n"
                    "   fld qword [rsp] ; load 64 bit float from stack\n"
                    "   faddp ; add and pop FPU\n"
                    "   fstp qword [rsp] ; store onto stack and pop FPU\n");
            } else if (params == (PARAM_1_DOUBLE | PARAM_2_INT)){
                fprintf(out, 
                    "   fld qword [rsp] ; load 64 bit float from stack\n"
                    "   add rsp, 8 ; pop from stack\n"
                    "   fild qword [rsp] ; load signed integer\n"
                    "   faddp ; add and pop FPU\n"
                    "   fstp qword [rsp] ; store onto stack and pop FPU\n");
            } else {
                crash("cannot compile " STRINGIFY(ADD) " instruction");
            }
        }break;
        case SUB: 
            params = inst->v.i;
        {
            if(params == PARAM_1_INT){
                fprintf(out, 
                    "   pop rax\n"
                    "   pop rcx\n"
                    "   sub rax, rcx\n"
                    "   push rax ; sub\n");
            } else if(params == PARAM_1_DOUBLE){
                fprintf(out, 
                    "   fld qword [rsp]; load first double from stack\n"
                    "   add rsp, 8\n"
                    "   fld qword [rsp] ; load second double from stack\n"
                    "   fsubp ; sub st0 from st1 and pop the FPU stack\n"
                    "   fstp qword [rsp] ; sub doubles\n");
            } else if (params == (PARAM_1_INT | PARAM_2_DOUBLE)){
                fprintf(out, 
                    "   fild qword [rsp] ; load signed integer\n"
                    "   add rsp, 8 ; pop from stack\n"
                    "   fld qword [rsp] ; load 64 bit float from stack\n"
                    "   fsubp ; sub and pop FPU\n"
                    "   fstp qword [rsp] ; store onto stack and pop FPU\n");
            } else if (params == (PARAM_1_DOUBLE | PARAM_2_INT)){
                fprintf(out, 
                    "   fld qword [rsp] ; load 64 bit float from stack\n"
                    "   add rsp, 8 ; pop from stack\n"
                    "   fild qword [rsp] ; load signed integer\n"
                    "   fsubp ; sub and pop FPU\n"
                    "   fstp qword [rsp] ; store onto stack and pop FPU\n");
            } else {
                crash("cannot compile " STRINGIFY(SUB) " instruction");
            }
        }break;
        case MUL: 
            params = inst->v.i;
        {
            if(params == PARAM_1_INT){
                fprintf(out, 
                    "   pop rax\n"
                    "   pop rcx\n"
                    "   imul rax, rcx\n"
                    "   push rax ; mul\n");
            } else if(params == PARAM_1_DOUBLE){
                fprintf(out, 
                    "   fld qword [rsp]; load first double from stack\n"
                    "   add rsp, 8\n"
                    "   fld qword [rsp] ; load second double from stack\n"
                    "   fmulp ; mul st0 and st1 and pop the FPU stack\n"
                    "   fstp qword [rsp] ; mul doubles\n");
            } else if (params == (PARAM_1_INT | PARAM_2_DOUBLE)){
                fprintf(out, 
                    "   fild qword [rsp] ; load signed integer\n"
                    "   add rsp, 8 ; pop from stack\n"
                    "   fld qword [rsp] ; load 64 bit float from stack\n"
                    "   fmulp ; mul and pop FPU\n"
                    "   fstp qword [rsp] ; store onto stack and pop FPU\n");
            } else if (params == (PARAM_1_DOUBLE | PARAM_2_INT)){
                fprintf(out, 
                    "   fld qword [rsp] ; load 64 bit float from stack\n"
                    "   add rsp, 8 ; pop from stack\n"
                    "   fild qword [rsp] ; load signed integer\n"
                    "   fmulp ; mul and pop FPU\n"
                    "   fstp qword [rsp] ; store onto stack and pop FPU\n");
            } else {
                crash("cannot compile " STRINGIFY(MUL) " instruction");
            }
        }break;
        case DIV: 
            params = inst->v.i;
        {
            if(params == PARAM_1_INT){
                fprintf(out, 
                    "   pop rax\n"
                    "   pop rcx\n"
                    "   cqo ; sign expand rax into rdx\n"
                    "   idiv rcx\n"
                    "   push rax ; div, quoitent in the rax\n");
            } else if(params == PARAM_1_DOUBLE){
                fprintf(out, 
                    "   fld qword [rsp]; load first double from stack\n"
                    "   add rsp, 8\n"
                    "   fld qword [rsp] ; load second double from stack\n"
                    "   fdivp ; div st0 by st1 and pop the FPU stack\n"
                    "   fstp qword [rsp] ; div doubles\n");
            } else if (params == (PARAM_1_INT | PARAM_2_DOUBLE)){
                fprintf(out, 
                    "   fild qword [rsp] ; load signed integer\n"
                    "   add rsp, 8 ; pop from stack\n"
                    "   fld qword [rsp] ; load 64 bit float from stack\n"
                    "   fdivp ; div and pop FPU\n"
                    "   fstp qword [rsp] ; store onto stack and pop FPU\n");
            } else if (params == (PARAM_1_DOUBLE | PARAM_2_INT)){
                fprintf(out, 
                    "   fld qword [rsp] ; load 64 bit float from stack\n"
                    "   add rsp, 8 ; pop from stack\n"
                    "   fild qword [rsp] ; load signed integer\n"
                    "   fdivp ; div and pop FPU\n"
                    "   fstp qword [rsp] ; store onto stack and pop FPU\n");
            } else {
                crash("cannot compile " STRINGIFY(MUL) 
                " instruction, inproper type parameters (%lld:%lld)", inst->line, inst->col);
            }
        } break;
        case MOD: {
            // params = inst->v.i;
            // if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT) )
            // {
                fprintf(out, 
                "   pop rax\n"
                    "   pop rcx\n"
                    "   cqo ; sign expand rax into rdx\n"
                    "   idiv rcx\n"
                    "   push rdx ; mod, remainder in the rdx\n"
                );
            // } else{
            //     crash("cannot compile " STRINGIFY(MOD) 
            //     " instruction, inproper type parameters (%lld:%lld)", inst->line, inst->col);
            // }
        } break;
        case NOT: {
            fprintf(out, 
                "   pop rax\n"
                "   normalize rax\n"
                "   not rax\n"
                "   and rax, 1 ; logical not\n"
                "   push rax\n");
        } break;
        case OR: {
            fprintf(out, 
            "   pop rax\n"
                "   normalize rax\n"
                "   pop rcx\n"
                "   normalize rcx\n"
                "   or rax, rcx\n"
                "   push rax ; logical or\n");
        } break;
        case AND: {
            fprintf(out, 
            "   pop rax\n"
                "   normalize rax\n"
                "   pop rcx\n"
                "   normalize rcx\n"
                "   and rax, rcx\n"
                "   push rax ; logical and\n");
        } break;
        case IF: {
            size_t if_inst_id = inst_pos;

            // check condition
            fprintf(out, 
            "   pop rax\n"
                "   normalize rax\n"
                "   cmp rax, 1 ; if condition\n"
                "   jne __if%lld ; jump if false\n", if_inst_id);

            if(inst_pos++ < instruction_amount) {
                fprintf(out, "   ; true branch\n");
                Instruction *next = &inst_arr[inst_pos];
                emit_instruction(next, out);
            }
            // post condition
            fprintf(out, 
            "   __if%lld: ; else branch\n", if_inst_id);

        } break;
        case PRINT: {
            int argn = inst->v.i;
            emit_print(argn, out);
        } break;
        case ASSERT: {
            compile_assert = true;
            char *str = inst->v.string;
            StringNode node = {
                .next = NULL,
                .num = inst_pos,
                .str = str,
            };
            StringList_append(&stringList, node);

            fprintf(out, 
                "   push str_%lld ; push assert str\n"
                "   call qword _assert\n"
                "   add rsp, 2 * 8 ; cleanup the arguments\n", node.num);

        } break;
        case SWAP: {
            fprintf(out, 
            "   pop rax\n"
            "   pop rcx\n"
            "   xchg rax, rcx\n"
            "   push rax\n"
            "   push rcx ; swap\n");
        } break;
        default:
            crash("compile error: instruction not supported (%lld:%lld)\n", inst->line, inst->col);
    }
}

void emit_print(int argn, FILE* out){
    //fprintf(stdout, "argn: %d\n", argn);
    if(argn == 0){
        fprintf(out, 
        "   pop rcx ; fmt string\n"
            "   sub rsp, 4 * 8 ; reserve stack for call\n"
            "   call [printf]\n"
            "   add rsp, 4 * 8 ; cleanup stack\n");
    } else if (argn == 1) {
        fprintf(out, 
        "   mov rax, rsp ; save rsp\n"
            "   sub rsp, 2 * 8 ; reserve space for last 2 empty args\n"
            "   push qword [rax + 1 * 8]; push arg 2\n"
            "   mov rdx, [rsp] ; load arg 2\n"
            "   push qword [rax] ; push arg 1\n"
            "   mov rcx, [rsp] ; load arg 1\n"
            "   call [printf]\n"
            "   add rsp, 5 * 8 ; cleanup stack including original fmt\n");
    } else if (argn == 2) {
        fprintf(out, 
        "   mov rax, rsp ; save rsp\n"
            "   sub rsp, 8 ; reserve stack for 4th empty argument\n"
            "   push qword [rax + 2 * 8] ; push arg 3\n"
            "   mov r8, [rsp] ; move arg 3\n"
            "   push qword [rax + 1 * 8] ; push arg 2\n"
            "   mov rdx, [rsp] ; move arg 2\n"
            "   push qword [rax] ; push arg 1\n"
            "   mov rcx, [rsp] ; move arg 1\n"
            "   call [printf]\n"
            "   add rsp, 5 * 8 ; cleanup stack including original fmt\n");
    } else if (argn == 3) {
        fprintf(out, 
        "   mov rax, rsp ; save rsp\n"
            "   push qword [rax + 3 * 8] ; push arg 4\n"
            "   mov r9, [rsp] ; load arg 4\n"
            "   push qword [rax + 2 * 8] ; push arg 3\n"
            "   mov r8, [rsp] ; load arg 3\n"
            "   push qword [rax + 1 * 8] ; push arg 2\n"
            "   mov rdx, [rsp] ; load arg 2\n"
            "   push qword [rax] ; push arg 1\n"
            "   mov rcx, [rsp] ; load arg 1\n"
            "   call [printf]\n"
            "   add rsp, 5 * 8 ; cleanup stack inluding original fmt\n");
    } else {
        fprintf(out, "   mov rax, rsp ; save rsp\n");
        for (int i = argn; i > 3; i--) {
            fprintf(out, 
            "   push qword [rax + %d * 8] ; push %dth arg\n", i, i);
        }
        fprintf(out, 
            "   push qword [rax + 3 * 8] ; push arg 4\n"
            "   mov r9, [rsp] ; load arg 4\n"
            "   push qword [rax + 2 * 8] ; push arg 3\n"
            "   mov r8, [rsp] ; load arg 3\n"
            "   push qword [rax + 1 * 8]; push arg 2\n"
            "   mov rdx, [rsp] ; load arg 2\n"
            "   push qword [rax] ; push arg 1\n"
            "   mov rcx, [rsp] ; load arg 1\n"
            "   call [printf]\n"
            "   add rsp, %d * 8 ; cleanup stack including original fmt\n", argn + 2);
    }
}