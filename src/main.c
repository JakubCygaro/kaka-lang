#include <corecrt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include "errors.h"
#include "parser.h"
#include <stdbool.h>

#define MAXSTACK 1000

Value stack[MAXSTACK];
long int pos;

void print_stack(){
    for(int i = 0; i < MAXSTACK; i++)
        Value_print(&stack[i]);
}

void push_value(Value v){
    if (pos >= MAXSTACK)
        err_print("stack overflowed");
    stack[pos++] = v;
}
Value pop_value(){
    if (pos < 0)
        err_print("empty stack");
    return stack[--pos];
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

int main(int argc, char** args){

    if (argc != 2)
        err_print("source file path was not provided.");
    check_source_ext(args[1]);



    FILE* source_file;
    errno_t error = fopen_s(&source_file, args[1], "r");
    if (error != 0){
        crash_on_error(error);
    }
    size_t instruction_amount;
    Instruction* inst = parse_from_file(source_file, &instruction_amount);

    for(size_t i = 0; i < instruction_amount; i++){
        excecute_instruction(&inst[i]);
    }
    //print_stack();
    fclose(source_file);
    free(inst);
    dealloc_values();
    return 0;
}
void excecute_instruction(Instruction* inst){
    switch (inst->c_type) {
        case PUSH:
            push_value(inst->v);
            break;
        case POP:
            pop_value();
            break;
        case PRINT:
            Value v = pop_value();
            Value_print(&v);
            push_value(v);
            break;
        case ADD:
            Value a = pop_value();
            Value b = pop_value();
            Value res;

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
            else 
                err_print("invalid addition operands");

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
                res.i = strcmp(a.string, b.string) == true;
            }
            else 
                err_print("invalid equation operands");
            
            push_value(res);
            break;
        case GREAT:
            b = pop_value();
            a = pop_value();
            if(a.v_type == INT && b.v_type == INT){
                res.v_type = INT;
                res.i = a.i > b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.v_type = INT;
                res.i = (double)a.i > b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.v_type = INT;
                res.i = a.d > (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.v_type = INT;
                res.i = a.d > b.d;
            }
            else 
                err_print("invalid equation operands");
            
            push_value(res);
            break;
        case GREATEQ:
            b = pop_value();
            a = pop_value();
            if(a.v_type == INT && b.v_type == INT){
                res.v_type = INT;
                res.i = a.i >= b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.v_type = INT;
                res.i = (double)a.i >= b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.v_type = INT;
                res.i = a.d >= (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.v_type = INT;
                res.i = a.d >= b.d;
            }
            else 
                err_print("invalid equation operands");
            
            push_value(res);
            break;
        case LESS:
            b = pop_value();
            a = pop_value();
            if(a.v_type == INT && b.v_type == INT){
                res.v_type = INT;
                res.i = a.i < b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.v_type = INT;
                res.i = (double)a.i < b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.v_type = INT;
                res.i = a.d < (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.v_type = INT;
                res.i = a.d < b.d;
            }
            else 
                err_print("invalid equation operands");
            
            push_value(res);
            break;
        case LESSEQ:
            b = pop_value();
            a = pop_value();
            if(a.v_type == INT && b.v_type == INT){
                res.v_type = INT;
                res.i = a.i <= b.i;
            }
            else if (a.v_type == INT && b.v_type == DOUBLE){
                res.v_type = INT;
                res.i = (double)a.i <= b.d;
            }
            else if (a.v_type == DOUBLE && b.v_type == INT){
                res.v_type = INT;
                res.i = a.d <= (double)b.i;
            }
            else if (a.v_type == DOUBLE && b.v_type == DOUBLE){
                res.v_type = INT;
                res.i = a.d <= b.d;
            }
            else 
                err_print("invalid equation operands");
            
            push_value(res);
            break;
    }
}

void check_source_ext(char* path){
    if (strspn(path, ".kaka") == 0)
        err_print("the specified file is not a .kaka file");
}
