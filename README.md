# kaka-lang
This is a simple interpreted stack-based programming language written in C.

I wrote this for fun and also to test out what I've learn about C so far, this is by no means a serious project.

## Compiling

### Requirements
- gcc

### Windows

Just run the `build.bat` script.

### Linux

Compile all the source files in the `src` directory using `gcc`.

## Data types

Kaka supports 3 variable types:
- `int`
- `double`
- `string`

## Commands
- `push`: push a value onto the stack
- `pop`: remove a value from the stack
- `add`: add two values 
- `sub`: subtract two values
- `mul`: multiply two values
- `div`: divide two values
- `mod`: divide two values and push the remainder
- `cmp`: compare two values and push the result (1 for equality, 0 for inequality)
- `great`: compare two values to check which is greater
- `greateq`: compare two values to chech which is greater or equal
- `less`: compare two values to check which is smaller
- `lesseq`: compare two values to check which is smaller or equal
- `int`: cast a value from `double` to `int`
- `double`: cast a value from `int` to `double`
- `clone`: clone a value
- `print`: print a value from the stack without consuming it
- `lab`: create a label
- `jmp`: jump to a label
- `if`: if the value on the stack is equal to 1, excecute the next command, otherwise skip it, consume the value in the process
- \_stack: print the stack without consuming any value

## Examples

Basic Arithmetic:
```
# addition
push 10
push 10
add
print
# 20 is printed

# subtraction
push 10
push 5
sub
print
# 5 is printed

# multiplication
push 10
push 2
mul
print
# 20 is printed

# division
push 10
push 5
div
print
# 2.000000 is printed

# modulo
push 11
push 10
mod
print
# 1 is printed
```

Logical Operations
```
push 0
push 1
and
print
# 0 (false) is printed

push 0
push 1
or
print
# 1 (true) is printed

push 0
not
print
pop
# 1 (true) is printed
```

Comparing Values:
```
# equality
push 10
push 100
cmp
print
# 0 (false) is printed

# greater than
push 100
push 10
great
print
# 1 (true) is printed

# greater or equal to
push 10
push 10
greateq
print
# 1 (true) is printed

# less than
push 10
push 100
less
print
# 1 (true) is printed

# less or equal to
push 10
push 10
lesseq
print
# 1 (true) is printed
```
Casting Values:
```
push 10
double
print
# 10.000000 is printed

push 1.0
int
print
# 1 is printed
```

Labels:
```
push "infinite loop"
label "start" # create a label with a unique name
print
jmp "start" # jump to a label
```

If Statement:
```
push "true"
push 0 # (false)
if # excecute the next command if the value on the stack is equal to 1 (true)
print
push "false"
print
# "false" is printed
```



