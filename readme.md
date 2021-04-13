# Testcases
We got many type of testcases. They will test both vm_x2017 and objdump_x2017

Type 1: bad/broken binary file

Type 2: endless call

Type 3: stack overflow / deep frame

Type 4: pointers!

Type 5: vm run time errors

Type 6: normal cases
# Error Handling
The register 4 in the vm is use for storing status and also error code.

reg[4]==0: no error

reg[4]==1: program return and terminated

reg[4]==2: program terminated without return

reg[4]==3: stack overflow

reg[4]==4: Invalid access on registers

reg[4]==5: Write on value type

reg[4]==6: Address type error

reg[4]==7: invalid function call

# Files
objdump_x2017.c: program for objdump_x2017

vm_x2017.c: program for vm_x2017

fetch_x2017.c: functions needed for fetch the code from binary file

fetch_x2017.h: header file for fetch_x2017.c

# Program structures

## function table
index: function label for vm_x2017, appearance order for objdump_x2017

[0]: appearance order for vm_x2017, function label for objdump_x2017, -1 if not exist / uninitialized

[0]: instruction count, -1 if not exist / uninitialized

## symbol table
index: stack number

[0]: appearance order, reversed

## code space
1D index: function label for vm_x2017, appearance order for objdump_x2017

2D index: instruction

[][][0]: opcode

[][][1]: first value type, 0 if not exist

[][][2]: first value, 0 if not exist

[][][3]: second value type, 0 if not exist

[][][4]: second value, 0 if not exist

[][][5]: check exist val, 0 if not exist and 1 if instruction exist


# VM Runtime structures

register 4: status code / SC

register 5: stack top pointer / STP

register 6: stack base pointer / SBP
