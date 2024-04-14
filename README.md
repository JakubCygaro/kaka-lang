# kaka-lang
___"Kaka is you"___

This is a simple interpreted (and now also compiled) stack-based programming language written in C.

I wrote this for fun and also to test out what I've learn about C so far, this is by no means a serious project.

## Compiling

### Requirements
- gcc

### Windows-Interpreter

Just run the `build.bat` script. Alternatively build all files in the `src` directory like with the linux version.

### Windows-Compiler

Run the `build_compiler.bat` stript or pass the `COMPILER` flag while building the interpreter.

### Linux-Interpreter

Compile all the source files in the `src` directory using `gcc`.

### Linux-Compiler

Not supported (Maybe I'll get to it sometime in the future)

## Running

### Interpreter

Run the executable in command line and provide a path to a `.kaka` file as the argument:

`.\kaka.exe .\file.kaka`

### Compiler

Run the executable in command line and provide a path to a `.kaka` file as the argument:

`.\kaka.exe .\file.kaka`

You can also supply the `-p` flag to preserve the intermediate assembly file.

`.\kaka.exe -p .\file.kaka`

files `a.exe` and `a.exe.compilation.asm` will appear.

## Data types

Kaka supports 3 variable types:
- `int` - 64-bit signed integer
- `double` - 64-bit floating point precision number
- `string` - ASCII character array

## Compiler (new!)

Kaka source can now be compiled for 64-bit Windows. You have to compile the compiler separately (see sections above). The compiler is using the FASM assembler as such you have to have it installed and avaliable on the path. The compiler also uses the MSVCR library for `print`s, so if you somehow don't have it on your system compilations will fail.

[Fasm Website](https://flatassembler.net/)

Compiled kaka is less safe when it comes to programmer errors. There is almost no error checking on the compiler's side so any error is on you. There are also type requirements when trying to compile kaka instructions (explained below).

## Type annotations (new!)

With the compiler come type annotations for cerain instructions (they will be all marked in the section below). Since the compiler cannot determine the type of instruction arguments that are taken from the stack, you as the programmer have to provide them yourself:

```
    4 4 add int int "%d" print 1
```

In the above example the `add` instruction is annotated with two `int` keywords, these inform the compiler that while compiling the instruction both its arguments taken from the stack have to be treated as 64-bit integers. When all instruction arguments have the same type you can ommit the second annotation. Other possible variants of type annotations are as follows:

`4 4 add int # int`
`4.0 4 add int double`
`4 4.0 add double int`
`4.0 4.0 add double # double`

Since the cast operator shares the same keyword as the type parameter, when there is ambiguity in your code you can use the `;` (empty statement) operator to separate type annotations from type castings like so:

`4 4 mul int ; double "%f" print 1`

## Empty statement operator (new!)

The empty statement operator `;` is used to tell the parser to stop parsing whatever it was parsing at the moment. In practice it's used to break up ambiguous parts of code or provide a 'null' parameter to an instruction. It has no other effect.

`"Hello Kaka" print ; # a print instruction with 0 format parameters`

## New print syntax and format strings (new!)

`print` instructions now are capable of printing values from the stack. You first have to supply the instruction a proper format string as an argument with appropriate format specifiers:
- %d - for integers
- %f - for doubles
- %s - for strings

and then provide the format argument amount as the parameter, after the `print` instruction like so:

` 1 2 3 4 "%d %d %d %d\n" print 4`

Strings now also support escape sequences.

## Assert (new!)

The `assert` instruction can now be used to exit the program if a condition is not met. The instruction takes a value from the stack and if it's equal to 0 (false) prints a message supplied as the parameter and exits the program.

`2 3 add 4 cmp int assert "2 plus 3 is not 4!\n"` 

## Implicit push

Values can be pushed without using the `push` keyword, like so:


```
push 4
4
_stack
# 4
# 4
# is printed
```

## Instruction
Instruction parameters are specified in `()`, while string parameters are additionally enclosed in quotation marks `("text")`.

- `push`: push a value onto the stack
- `pop`: remove a value from the stack
- `add (type) (type)`: add two values 
- `sub (type) (type)`: subtract two values
- `mul (type) (type)`: multiply two values
- `div (type) (type)`: divide two values
- `mod`: divide two values and push the remainder, the compiler always expects the values to be of type `int`
- `and`: logical AND, two `int` values expected
- `or`: logical OR, two `int` values expected
- `not`: logical NOT, two `int` values expected
- `cmp (type) (type)`: compare two values and push the result (1 for equality, 0 for inequality)
- `great (type) (type)`: compare two values to check which is greater
- `greateq (type) (type)`: compare two values to chech which is greater or equal
- `less (type) (type)`: compare two values to check which is smaller
- `lesseq (type) (type)`: compare two values to check which is smaller or equal
- `int`: cast a value from `double` to `int`
- `double`: cast a value from `int` to `double`
- `clone`: clone a value
- `print (arg-num)`: print a value from the stack without consuming it, this instruction takes as a parameter the number of parameters for the format string taken from the stack
- `lab ("name")`: create a label
- `jmp ("name")`: jump to a label
- `if`: if the value on the stack is equal to 1, excecute the next command, otherwise skip it, consume the value in the process
- `swap`: swaps the two values on top of the stack
- `assert ("msg")`: checks whether a value on the stack is not equal to 0 and otherwise exits the program
- \_stack: print the stack without consuming any value (from the top to the bottom), **this instruction is not supported by the compiler**

## Examples

### Basic Arithmetic:
```
# addition
push 10
push 10
add int int
"%d\n" print 1
# 20 is printed

# subtraction
push 5
push 10
sub int int
"%d\n" print 1
# 5 is printed

# multiplication
push 10
push 2
mul int int
"%d\n" print 1
# 20 is printed

# division
push 10
push 5
div int int
"%f\n" print 1
# 2.000000 is printed

# modulo
push 10
push 11
mod
"%d\n" print 1
# 1 is printed
```

### Logical Operations
```
push 0
push 1
and
"%d\n" print 1
# 0 (false) is printed

push 0
push 1
or
"%d\n" print 1
# 1 (true) is printed

push 0
not
"%d\n" print 1
# 1 (true) is printed
```

### Comparing Values:
```
# equality
push 10
push 100
cmp int int
"%d\n" print 1
# 0 (false) is printed

# greater than
push 100
push 10
great int int
"%d\n" print 1
# 0 (false) is printed

# greater or equal to
push 10
push 10
greateq int int
"%d\n" print 1
# 1 (true) is printed

# less than
push 10
push 100
less int int
"%d\n" print 1
# 0 (false) is printed

# less or equal to
push 10
push 10
lesseq int int
"%d\n" print 1
# 1 (true) is printed

```
### Casting Values:
```
push 10
double
"%f\n" print 1
# 10.000000 is printed

push 1.0
int
"%d\n" print 1
# 1 is printed
```

### Labels:
```
push "infinite loop"
lab "start\n" # create a label with a unique name
print
jmp "start\n" # jump to a label
```

### If Statement:
```
push "true\n"
push 0 # (false)
if # excecute the next command if the value on the stack is equal to 1 (true)
print 1
push "false\n"
print 1
# "false" is printed
```

### \_stack:
```
push 1
push 2
push 3
_stack
# 3
# 2
# 1
```

### assert:
```
push 1
push 2
cmp int int
assert "Not equal\n"
# "Not equal" is printed

```



