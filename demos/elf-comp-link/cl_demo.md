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
    


#### Static Linked Deep Dive
2.  Lets examine the ELF headers
    ```bash
    readelf -h ./cl_demo.o
    readelf -h ./math_stuff.o
    ```