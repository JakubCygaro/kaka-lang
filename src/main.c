#include <corecrt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "errors.h"
#include "parser.h"
#include <stdbool.h>
#include "label.h"

#define MAXSTACK 1000

static Instruction* inst;
static size_t instruction_amount;
static size_t inst_pos = 0;
Value stack[MAXSTACK];
long int pos = 0;

void print_stack(){
    for(long int i = 0; i < pos; i++)
        Value_print(&stack[i]);
}
void print_full_stack(){
    for(long int i = 0; i < MAXSTACK; i++)
        Value_print(&stack[i]);
}

void push_value(Value v){
    if (pos >= MAXSTACK){

        err_print("stack overflowed");
    }
    stack[pos++] = v;
}
Value pop_value(){
    if (pos < 0){
        //CommandType_print(inst[inst_pos].c_type);
        err_print("empty stack");
    }
    return stack[--pos];
}

LabelMap label_map;

void setup_labels(){
    label_map = LabelMap_new();
    while(inst_pos < instruction_amount){
        switch (inst[inst_pos].c_type) {
            case LABEL:
                bool out = LabelMap_insert(&label_map, inst[inst_pos].v.string, inst_pos);
                if(!out)
                    err_print("redeclaration of label `%s`", inst[inst_pos].v.string);
            
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
                    err_print("no such label `%s`", inst[inst_pos].v.string);
                inst[inst_pos].jump_dest = ret;
            default:
            ;
        }
        inst_pos++;
    }
    inst_pos = 0;
}

void check_source_ext(char*);

void excecute_instruction(Instruction* inst);

void dealloc_values(){
    for (int i = 0; i < MAXSTACK; i++){
        Value v = stack[i];
        if(v.v_type != STRING)
            continue;
        free(v.string);
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


    FILE* source_file;
    errno_t error = fopen_s(&source_file, args[1], "r");
    if (error != 0){
        crash_on_error(error);
    }
    inst = parse_from_file(source_file, &instruction_amount);


    setup_labels();

    while(inst_pos < instruction_amount){
        excecute_instruction(&inst[inst_pos]);

        inst_pos++;
    }
    fclose(source_file);
    dealloc_values();
    dealloc_inst();
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
                err_print("invalid cast exception, line: %lld", inst_pos + 1);
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
                err_print("invalid cast exception, line: %lld", inst_pos + 1);
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
                err_print("invalid addition operands");
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
                res.d = a.i - b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.v_type = DOUBLE;
                res.d = a.d - b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.v_type = DOUBLE;
                res.d = a.d - b.d;
            }
            else 
                err_print("invalid subtraction operands");

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
                err_print("invalid multiplication operands");

            push_value(res);
            break;
        case DIV:
            Value divisor = pop_value();
            a = pop_value();
            if(divisor.v_type == INT)
                if(divisor.i == 0)
                    err_print("zero divisor");
            if(divisor.v_type == DOUBLE)
                if(divisor.d == 0)
                    err_print("zero divisor");

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
                err_print("invalid division operands");

            push_value(res);
            break;
        case MOD:
            a = pop_value();
            b = pop_value();

            if(a.v_type == INT && b.v_type == INT){
                res.v_type = INT;
                res.i = a.i % b.i;
            }
            else 
                err_print("invalid modulo operands");
            
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
                err_print("invalid comparison operands");
            
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
                err_print("invalid great operands");
            
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
                err_print("invalid greateq operands");
            
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
                err_print("invalid less operands");
            
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
                err_print("invalid lesseq operands");
            
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
                err_print("invalid if statement argument");
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
    }
}

void check_source_ext(char* path){
    if (strspn(path, ".kaka") == 0)
        err_print("the specified file is not a .kaka file");
}
