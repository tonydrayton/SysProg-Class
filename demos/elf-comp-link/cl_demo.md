## CL_DEMO

#### Introduction

1. To build run `make` or `make static`, the makefile should do everything.  You can also run `make clean` to clean things up.

2. To start understanding things lets look at both the statically and dynamically linked version fo this program by running `make static` and `make`.

- Running `make`:
    ```bash
    bsm23@code:~/SysProg-Class/demos/elf-comp-link$ touch *.c
    bsm23@code:~/SysProg-Class/demos/elf-comp-link$ make
    gcc -Wall -g -c cl_demo.c
    gcc -Wall -g -c math_stuff.c
    gcc  -o cl_demo cl_demo.o math_stuff.o
    bsm23@code:~/SysProg-Class/demos/elf-comp-link$ ls -l ./cl_demo
    -rwxr-xr-x 1 bsm23 bsm23 72600 Jan  2 08:56 ./cl_demo
    ```
- Running `make static`:
    ```bash
    bsm23@code:~/SysProg-Class/demos/elf-comp-link$ touch *.c
    bsm23@code:~/SysProg-Class/demos/elf-comp-link$ make static
    gcc -Wall -g -c cl_demo.c
    gcc -Wall -g -c math_stuff.c
    gcc -static -o cl_demo cl_demo.o math_stuff.o
    bsm23@code:~/SysProg-Class/demos/elf-comp-link$ ls -l ./cl_demo
    -rwxr-xr-x 1 bsm23 bsm23 642128 Jan  2 09:01 ./cl_demo
    ```
    Notice that the statically linked version of the program comes in at 642,128 bytes, compared with the standard (dynamically) linked version that comes in much smaller at 72,600 bytes.  This is almost `9x` smaller.  We will get into this shortly.


#### Static Linked Deep Dive

For this part of the demo we will be looking into the statically linked version of this program.  Run `touch *.c` and then `make static`.  Now lets dig into what we have:

1.  Lets examine the ELF headers:
    ```bash
    readelf -h ./cl_demo.o
    readelf -h ./math_stuff.o
    ```
    Notice both of these files have the type of `REL(Relocatable file)` with an `Entry point address = 0`.  ELF files that are of type `REL` cannot be executed.

2. Now lets look at the sections:
```bash
readelf -s ./cl_demo.o
readelf -s ./math_stuff.o
```
Notice that for `cl_demo.o`:
```bash
    18: 0000000000000000    44 FUNC    GLOBAL DEFAULT    1 isodd
    19: 000000000000002c   124 FUNC    GLOBAL DEFAULT    1 main
    20: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND my_add
    21: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf
    22: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND puts
    23: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND exit
```


