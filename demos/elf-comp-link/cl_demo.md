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

2. Now lets look at the sections, specifically the symbol table (`-s`):
    ```bash
    readelf -s ./cl_demo.o
    readelf -s ./math_stuff.o
    ```
    Notice that for `cl_demo.o` (im eliminating the gcc generated symbols):
    ```bash
    Symbol table '.symtab' contains 24 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS cl_demo.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    3 .data
     4: 0000000000000000     0 SECTION LOCAL  DEFAULT    4 .bss
     5: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    1 $x
     6: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     7: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    5 $d
     8: 0000000000000000     0 SECTION LOCAL  DEFAULT    6 .debug_info
     9: 0000000000000000     0 SECTION LOCAL  DEFAULT    8 .debug_abbrev
    10: 0000000000000000     0 SECTION LOCAL  DEFAULT    9 .debug_aranges
    11: 0000000000000000     0 SECTION LOCAL  DEFAULT   11 .debug_line
    12: 0000000000000000     0 SECTION LOCAL  DEFAULT   13 .debug_str
    13: 0000000000000000     0 SECTION LOCAL  DEFAULT   14 .debug_line_str
    14: 0000000000000000     0 SECTION LOCAL  DEFAULT   16 .note.GNU-stack
    15: 0000000000000014     0 NOTYPE  LOCAL  DEFAULT   17 $d
    16: 0000000000000000     0 SECTION LOCAL  DEFAULT   17 .eh_frame
    17: 0000000000000000     0 SECTION LOCAL  DEFAULT   15 .comment
    18: 0000000000000000    44 FUNC    GLOBAL DEFAULT    1 isodd
    19: 000000000000002c   124 FUNC    GLOBAL DEFAULT    1 main
    20: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND my_add
    21: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf
    22: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND puts
    23: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND exit
    ```

    All of the symbols (aka functions) that are included in this file like `main()` and `isodd()` has a size, and the `1` means that the implementation is in Section 1 of the ELF file.  If you go to the top you will see that `SECTION 1` is the `.text` section.  This is where the machine code for these functions is located.  Notice that the functions `my_add()`, `printf()`, `puts()`, and `exit()` are marked as `UND` or undefined. Why?  Because these functions are not present in this object file, they will have to be resolved later by the linking process.

    **sidebar** where did `puts()` come from, we never call that function.  This is a gcc "optimization".  When we call `printf()`, the `printf()` function in the C runtime library is called if we use any sort of formatting in the call.  We did this here: `printf("The value of %d + %d is %d\n", a,b,c);`.  However, if we call `printf()` without any formatting the compiler replaces it with a simpler and faster call to `puts()` like we did here: `printf("The result is odd\n");`.  Back to the demo...

    Now lets disassemble main by running `objdump -d ./cl_demo.o`.  I am running on an ARM-based machine:

    ```bash
    000000000000002c <main>:
    2c:   a9be7bfd        stp     x29, x30, [sp, #-32]!
    30:   910003fd        mov     x29, sp
    34:   528000a0        mov     w0, #0x5                        // #5
    38:   b90017e0        str     w0, [sp, #20]
    3c:   52800280        mov     w0, #0x14                       // #20
    40:   b9001be0        str     w0, [sp, #24]
    44:   b9401be1        ldr     w1, [sp, #24]
    48:   b94017e0        ldr     w0, [sp, #20]
    4c:   94000000        bl      0 <my_add>
    50:   b9001fe0        str     w0, [sp, #28]
    54:   b9401fe3        ldr     w3, [sp, #28]
    58:   b9401be2        ldr     w2, [sp, #24]
    5c:   b94017e1        ldr     w1, [sp, #20]
    60:   90000000        adrp    x0, 0 <isodd>
    64:   91000000        add     x0, x0, #0x0
    68:   94000000        bl      0 <printf>
    6c:   b9401fe0        ldr     w0, [sp, #28]
    70:   94000000        bl      0 <isodd>
    74:   39004fe0        strb    w0, [sp, #19]
    78:   39404fe0        ldrb    w0, [sp, #19]
    7c:   12000000        and     w0, w0, #0x1
    80:   7100001f        cmp     w0, #0x0
    84:   540000a0        b.eq    98 <main+0x6c>  // b.none
    88:   90000000        adrp    x0, 0 <isodd>
    8c:   91000000        add     x0, x0, #0x0
    90:   94000000        bl      0 <puts>
    94:   14000004        b       a4 <main+0x78>
    98:   90000000        adrp    x0, 0 <isodd>
    9c:   91000000        add     x0, x0, #0x0
    a0:   94000000        bl      0 <puts>
    a4:   52800000        mov     w0, #0x0                        // #0
    a8:   94000000        bl      0 <exit>
    ```


    Notice that for `cl_demo.o`:
    ```bash
        16: 0000000000000000    40 FUNC    GLOBAL DEFAULT    1 my_add
    ```


