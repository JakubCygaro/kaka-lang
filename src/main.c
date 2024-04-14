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
#include "stringmap.h"
#include <string.h>
#include "util.h"
#include "args_parse.h"

#define MAXSTACK 100000
#define STRINGIFY(A) #A

static FILE* source_file;

bool check_ext(const char*, const char*);
void dealloc_values();
void dealloc_inst();
void crash(const char* fmt, ...);
void check_source_ext(char*);
void excecute_instruction(Instruction* inst);
void dealloc_value(Value* v);
void stack_printf(const char* fmt, Value values[], size_t val_amout);
Args setup_args(char** args, int argn);

#define COMPILER 1

#if COMPILER
void emit_instruction(Instruction* inst, FILE *out);
void compile();
void emit_print(long long int, FILE*);
static bool compile_assert = false;
#endif

static Instruction* inst_arr;
static size_t instruction_amount;
static size_t inst_pos = 0;
static int output_specified = 0;
static char* output_path = NULL;
static int preserve = 0;
static Value stack[MAXSTACK];
static long int pos = 0;
static LabelMap label_map;
static StringMap string_map;

void print_stack(){
    for(long long int i = pos-1; i >= 0; i--)
        Value_print(&stack[i]);
}
void print_full_stack(){
    for(long long int i = MAXSTACK-1; i >= 0; i--)
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
                    crash("redeclaration of label `%s` (%llu:%llu)", 
                        inst_arr[inst_pos].v.string, 
                        inst_arr[inst_pos].line, 
                        inst_arr[inst_pos].col);
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
                    crash("no `%s` label declared (%llu:%llu)", 
                        inst_arr[inst_pos].v.string,
                        inst_arr[inst_pos].line,
                        inst_arr[inst_pos].col);
                inst_arr[inst_pos].jump_dest = ret;
            default:
            ;
        }
        inst_pos++;
    }
    inst_pos = 0;
}
void dealloc_values(){
    for (long long int i = 0; i < MAXSTACK; i++){
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

Args setup_args(char** args, int argn){
    Args ret;
    ParseResult res = parse_args(args + 1, argn - 1, &ret);
    switch (res) {
        case NoSource: {
            err_print("soure file path was not provided.");
        }break;
        case InvalidFlags: {
            err_print("invalid flags.");
        }break;
        case TooManyArgs: {
            err_print("too many arguments.");
        }break;
        case UnknownFlag: {
            err_print("unknown flag");
        } break;
        case Success: {
            break;
        }
    }
    return ret;
}

int main(int argc, char** args){
    Args parsed_args = setup_args(args, argc);
    preserve = parsed_args.preserve;
    if(parsed_args.out_path != NULL){
        output_specified = 1;
        output_path = parsed_args.out_path;
    }
    check_source_ext(parsed_args.source_path);

    string_map = StringMap_new();

    errno_t error = fopen_s(&source_file, parsed_args.source_path, "r");
    if (error != 0){
        crash_on_error(error);
    }
    inst_arr = parse_from_file(source_file, &instruction_amount, &string_map);
    fclose(source_file);
    setup_labels();

#ifndef COMPILER
    while(inst_pos < instruction_amount){
        excecute_instruction(&inst_arr[inst_pos]);
        inst_pos++;
    }
    LabelMap_destroy(&label_map);
#else

    LabelMap_destroy(&label_map);
    compile();
#endif

    dealloc_values();
    dealloc_inst();
    free(inst_arr);
    StringMap_free(&string_map);
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
                long long argn = inst->v.i;
                Value args[argn];
                for (long long i = 0; i < argn; i++){
                    args[i] = pop_value();
                }
                stack_printf(fmt, args, argn);
                for (long long i = argn - 1; i >= 0; i--){
                    push_value(args[i]);
                }
            } break;
        case CAST_INT:
            v = pop_value();
            Value res;
            res.v_type = INT;
            if(v.v_type == DOUBLE){
                res.i = (long long)v.d;
            }
            else if(v.v_type == INT){
                res.i = v.i;
                push_value(res);
            }
            else {
                crash("invalid cast exception, (%llu:%llu)", inst->line, inst->col);
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
                crash("invalid cast exception, (%llu:%llu)", inst->line, inst->col);
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
                crash("invalid logical and aruments (%llu:%llu)", inst->line, inst->col);
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
                crash("invalid logical negation argument");
            }
            push_value(res);
            break;
        case ASSERT: {
            a = pop_value();
            if(a.v_type != INT){
                crash("Assertion failed: value not of type int (%llu:%llu)", inst->line, inst->col);
            }
            if(!a.i){
                crash("Assertion failed: \"%s\" (%llu:%llu)", inst->v.string, inst->line, inst->col);
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
                fprintf(stdout, "%lld", values[val_i++].i);
            }break;
            case 'f': {
                ensure_value_print(DOUBLE, &values[val_i]);
                fprintf(stdout, "%lf", values[val_i++].d);
            }break;
            case '%': {
                fprintf(stdout, "%%");
            }

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

void write_string_data(StringMapNode *node, void *data) {
    FILE* out = data;
    fprintf(out, 
        "str_%llu db ", node->hash);
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

    char *out_path = output_specified ? output_path : "a.exe";
    char *intermediate_path = push_str(out_path, ".compilation.asm");
    
    fopen_s(&output, intermediate_path, "w+");
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
        "   sub rsp, 8 ; align stack\n"
        "   fstcw [oldcw] ; store FPU flags\n"
        "   fwait\n"
        "   mov ax, [oldcw] ; load FPU flags to register\n"
        "   and ax, 0F3FFH ; zero out the RC flag\n"
        "   or ax, 0CC00h ; set RC to truncation\n"
        "   push rax ; push flags onto the stack\n"
        "   fldcw [rsp] ; load flags\n"
        "   add rsp, 8 ; cleanup flags from stack\n"
        "   xor rax, rax\n");

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
            "_assert: ; assert subprocedure\n"
            "   mov rax, rsp\n"
            "   mov rcx, [rax + 16] ; load int arg\n"
            "   cmp rcx, 0 ; check if false\n"
            "   jne .exit ; exit if not false\n"
            "   mov rcx, [rax + 8] ; load string arg\n"
            "   sub rsp, 4 * 8; reserve stack\n"
            "   call [printf] ; print assert failure message\n"
            "   mov rcx, 1\n"
            "   call [ExitProcess]\n"
            ".exit:\n"
            "   ret ; return\n");

    //data section
    fprintf(output, 
        "section '.data' data readable writable\n"
        "; COMPILER SECTION\n"
        "oldcw dw ?\n"
        "; STRING DATA SECTION\n");
    //string data section
    StringMap_foreach(&string_map, write_string_data, output);

    fclose(output);

    char *call1 = push_str("fasm ", intermediate_path);
    char *call2 = push_str(call1, " ");
    char *call3 = push_str(call2, out_path);

    //printf("call: %s\n", call3);
    if(system(call3) != 0){
        if(remove(intermediate_path)){
            fprintf(stderr, "Warning: could not delete the intermediate compilation file\n");
        }
        free(intermediate_path);
        free(call1);
        free(call2);
        free(call3);
        crash("assembler error, could not compile the intermediate file.");
    };

    if(!preserve){
        if(remove(intermediate_path)){
            fprintf(stderr, "Warning: could not delete the intermediate compilation file\n");
        }
    }

    free(intermediate_path);
    free(call1);
    free(call2);
    free(call3);
}

void emit_instruction(Instruction* inst, FILE *out){
    switch (inst->c_type) {
        case PUSH: {
            if(inst->v.v_type == NONE)
                crash("value not supported");
            if(inst->v.v_type == STRING){
                unsigned char *str = (unsigned char*)inst->v.string;
                StringMapNode *node = StringMap_get(&string_map, str);
                if(node == NULL){
                    crash("could not get string id for compilation\n");
                }
                fprintf(out, 
                    "   push str_%llu ; push str\n", node->hash);
            } else if (inst->v.v_type == DOUBLE) {
                fprintf(out, 
                    "   mov rax, %lf\n"
                    "   push rax ; push double\n", inst->v.d);
            }else {
                fprintf(out, 
                    "   push %lld ; push integer\n", inst->v.i);
            }
        }break;
        case POP: {
            fprintf(out, 
                "   add rsp, 8 ; pop\n");
        }break;
        case ADD: 
            int params = inst->v.i;
        {
            if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT)){
                fprintf(out, 
                    "   pop rax\n"
                    "   pop rcx\n"
                    "   add rax, rcx\n"
                    "   push rax ; add\n");
            } else if(params == PARAM_1_DOUBLE || params == (PARAM_1_DOUBLE | PARAM_2_DOUBLE)){
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
                crash("cannot compile " STRINGIFY(ADD) 
                    " instruction, missing type annotations (%llu:%llu)", inst->line, inst->col);
            }
        }break;
        case SUB: 
            params = inst->v.i;
        {
            if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT)){
                fprintf(out, 
                    "   pop rax\n"
                    "   pop rcx\n"
                    "   sub rax, rcx\n"
                    "   push rax ; sub\n");
            } else if(params == PARAM_1_DOUBLE || params == (PARAM_1_DOUBLE | PARAM_2_DOUBLE)){
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
                crash("cannot compile " STRINGIFY(SUB) 
                    " instruction, missing type annotations (%llu:%llu)", inst->line, inst->col);
            }
        }break;
        case MUL: 
            params = inst->v.i;
        {
            if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT)){
                fprintf(out, 
                    "   pop rax\n"
                    "   pop rcx\n"
                    "   imul rax, rcx\n"
                    "   push rax ; mul\n");
            } else if(params == PARAM_1_DOUBLE || params == (PARAM_1_DOUBLE | PARAM_2_DOUBLE)){
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
                crash("cannot compile " STRINGIFY(MUL) 
                    " instruction, missing type annotations (%llu:%llu)", inst->line, inst->col);
            }
        }break;
        case DIV: 
            params = inst->v.i;
        {
            if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT)){
                fprintf(out, 
                    "   pop rax\n"
                    "   pop rcx\n"
                    "   cqo ; sign expand rax into rdx\n"
                    "   idiv rcx\n"
                    "   push rax ; div, quoitent in the rax\n");
            } else if(params == PARAM_1_DOUBLE || params == (PARAM_1_DOUBLE | PARAM_2_DOUBLE)){
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
                crash("cannot compile " STRINGIFY(DIV) 
                    " instruction, missing type annotations (%llu:%llu)", inst->line, inst->col);
            }
        } break;
        case MOD: {
            fprintf(out, 
            "   pop rax\n"
                "   pop rcx\n"
                "   cqo ; sign expand rax into rdx\n"
                "   idiv rcx\n"
                "   push rdx ; mod, remainder in the rdx\n"
            );
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
                "   jne __if%llu ; jump if false\n", if_inst_id);

            if(inst_pos++ < instruction_amount) {
                fprintf(out, "   ; true branch\n");
                Instruction *next = &inst_arr[inst_pos];
                emit_instruction(next, out);
            }
            // post condition
            fprintf(out, 
            "   __if%llu: ; else branch\n", if_inst_id);

        } break;
        case LABEL: {
            fprintf(out, 
            ".%s: ; lab\n", inst->v.string);
        } break;
        case JUMP: {
            char *lab = inst->v.string;
            fprintf(out, 
            "   jmp .%s ; lab\n", lab);
        } break;
        case CLONE: {
            int count = inst->v.i;
            if(count <= 0) {
                crash("the argument for the CLONE instruction can only be a positive integer, (%llu:%llu)", 
                    inst->line, inst->col);
            }
            fprintf(out, 
            "   mov qword rax, [rsp] ; clone start\n");
            for (int i = 0; i < count; i++) {
                fprintf(out, 
                "   push rax\n");
            }
        } break;
        case CMP: {
            params = inst->v.i;
            if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT)) {
                fprintf(out, 
                "   pop rax\n"
                "   pop rcx\n"
                "   cmp rax, rcx\n"
                "   push 1\n"
                "   je @f\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; cmp\n");
            }
            else if (params == PARAM_1_DOUBLE || params == (PARAM_1_DOUBLE | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   je @f\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; cmp\n");
            }
            else if (params == (PARAM_1_INT | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fild qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   je @f\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; cmp\n");
            }
            else if (params == (PARAM_1_DOUBLE | PARAM_2_INT))  {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fild qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   je @f\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; cmp\n");
            } else {
                crash("cannot compile " STRINGIFY(CMP) 
                    " instruction, missing type annotations (%llu:%llu)", inst->line, inst->col);
            }
        } break;
        case GREAT: {
            params = inst->v.i;
            if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT)) {
                fprintf(out, 
                "   pop rax\n"
                "   pop rcx\n"
                "   cmp rax, rcx\n"
                "   push 1\n"
                "   ja @f\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == PARAM_1_DOUBLE || params == (PARAM_1_DOUBLE | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   jb @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == (PARAM_1_INT | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fild qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   jb @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == (PARAM_1_DOUBLE | PARAM_2_INT))  {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fild qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   jb @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            } else {
                crash("cannot compile " STRINGIFY(GREAT) 
                    " instruction, missing type annotations (%llu:%llu)", inst->line, inst->col);
            }
        } break; 
        case GREATEQ: {
            params = inst->v.i;
            if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT)) {
                fprintf(out, 
                "   pop rax\n"
                "   pop rcx\n"
                "   cmp rax, rcx\n"
                "   push 1\n"
                "   jae @f\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == PARAM_1_DOUBLE || params == (PARAM_1_DOUBLE | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   jbe @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == (PARAM_1_INT | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fild qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   jbe @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == (PARAM_1_DOUBLE | PARAM_2_INT))  {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fild qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   jbe @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            } else {
                crash("cannot compile " STRINGIFY(GREATEQ) 
                    " instruction, missing type annotations (%llu:%llu)", inst->line, inst->col);
            }
        } break;
        case LESS: {
            params = inst->v.i;
            if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT)) {
                fprintf(out, 
                "   pop rax\n"
                "   pop rcx\n"
                "   cmp rax, rcx\n"
                "   push 1\n"
                "   jb @f\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == PARAM_1_DOUBLE || params == (PARAM_1_DOUBLE | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   ja @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == (PARAM_1_INT | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fild qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   ja @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == (PARAM_1_DOUBLE | PARAM_2_INT))  {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fild qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   ja @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            } else {
                crash("cannot compile " STRINGIFY(LESS) 
                    " instruction, missing type annotations (%llu:%llu)", inst->line, inst->col);
            }
        } break;
        case LESSEQ: {
            params = inst->v.i;
            if(params == PARAM_1_INT || params == (PARAM_1_INT | PARAM_2_INT)) {
                fprintf(out, 
                "   pop rax\n"
                "   pop rcx\n"
                "   cmp rax, rcx\n"
                "   push 1\n"
                "   jbe @f\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == PARAM_1_DOUBLE || params == (PARAM_1_DOUBLE | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   jae @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == (PARAM_1_INT | PARAM_2_DOUBLE)) {
                fprintf(out, 
                "   fild qword [rsp]\n"
                "   add rsp, 8\n"
                "   fld qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   jae @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            }
            else if (params == (PARAM_1_DOUBLE | PARAM_2_INT))  {
                fprintf(out, 
                "   fld qword [rsp]\n"
                "   add rsp, 8\n"
                "   fild qword [rsp]\n"
                "   fcomip st, st1 \n"
                "   ffree st\n"
                "   mov qword [rsp], 1\n"
                "   jae @f ; since the values are compared in reverse order\n"
                "   mov qword [rsp], 0\n"
                "   @@: ; great\n");
            } else {
                crash("cannot compile " STRINGIFY(LESSEQ) 
                    " instruction, missing type annotations (%llu:%llu)", inst->line, inst->col);
            }
        } break;
        case CAST_DOUBLE: {
            fprintf(out, 
        "   fild qword [rsp] ; load value for double cast\n"
            "   fstp qword [rsp] ; cast complete\n");
        } break;
        case CAST_INT: {
            fprintf(out, 
        "   fld qword [rsp] ; load value for int cast\n"
            "   fistp qword [rsp] ; cast complete\n");
        } break;
        case PRINT: {
            int argn = inst->v.i;
            emit_print(argn, out);
        } break;
        case ASSERT: {
            compile_assert = true;
            char *str = inst->v.string;
            StringMapNode *node = StringMap_get(&string_map, (unsigned char*)str);
            fprintf(out, 
                "   push str_%llu ; push assert str\n"
                "   call qword _assert\n"
                "   add rsp, 2 * 8 ; cleanup the arguments\n", node->hash);

        } break;
        case SWAP: {
            fprintf(out, 
            "   pop rax\n"
            "   pop rcx\n"
            "   xchg rax, rcx\n"
            "   push rcx\n"
            "   push rax ; swap\n");
        } break;
        case PRINT_STACK: {
            fprintf(stderr, "Warning: The _stack instruction is interpreter exclusive, as such it won't be compiled and will have no effect during program execution (%llu:%llu)\n", inst->line, inst->col);
        } break;
        default:
            crash("compile error: instruction not supported (%llu:%llu)\n", inst->line, inst->col);
    }
}

void emit_print(long long argn, FILE* out){
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
        for (long long i = argn; i > 3; i--) {
            fprintf(out, 
            "   push qword [rax + %lld * 8] ; push %lldth arg\n", i, i);
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
            "   add rsp, %lld * 8 ; cleanup stack including original fmt\n", argn + 2);
    }
}