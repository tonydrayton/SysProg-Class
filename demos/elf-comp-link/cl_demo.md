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
    -rwxr-xr-x 1 bsm23 bsm23 642136 Jan  2 09:01 ./cl_demo
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

3.  Now lets disassemble main by running `objdump -d ./cl_demo.o`.  I am running on an ARM-based machine:

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

    Lets factor out the function calls, in ARM the instruction is `bl` that has the opcode `100101`:

    ```bash
    000000000000002c <main>:
    4c:   94000000        bl      0 <my_add>
    68:   94000000        bl      0 <printf>
    70:   94000000        bl      0 <isodd>
    90:   94000000        bl      0 <puts>
    a0:   94000000        bl      0 <puts>
    a8:   94000000        bl      0 <exit>
    ```

    The machine code for all of these calls is exactly the same thing: `0x94000000`.  In binary that is `0b100101 <26 more zeros>`.  Thus, the opcode is `bl (100101)` but the jump address is unknown so its replaced with all zeros.  If you have an Intel machine, or run on tux, the same thing happens, but this time the assembler instruction is `call` vs `bl`.  Running on tux, all of the function calls in `cl_demo.o`:

    ```bash
    000000000000002c <main>:
    47:	e8 00 00 00 00       	call   4c <main+0x29>
    69:	e8 00 00 00 00       	call   6e <main+0x4b>
    73:	e8 00 00 00 00       	call   78 <main+0x55>
    8b:	e8 00 00 00 00       	call   90 <main+0x6d>
    9c:	e8 00 00 00 00       	call   a1 <main+0x7e>
    a6:	e8 00 00 00 00       	call   ab <main+0x88>
    ```
    Just like the ARM version, the machine code for Intel/AMD is `e6` for the `call` opcode and all zeros for where to branch because its undefined in a `REL` ELF file (the format for .o object files).

4.  Now lets look at the linked version `cl_demo`.  Since this file is statically linked, the linker basically combines all of the `.text` and `.data` sections to create an executable ELF file.  Running:

    ```bash
    ➜  elf-comp-link git:(main) readelf -h ./cl_demo      
    ELF Header:
    Magic:   7f 45 4c 46 02 01 01 03 00 00 00 00 00 00 00 00 
    Class:                             ELF64
    Data:                              2's complement, little endian
    Version:                           1 (current)
    OS/ABI:                            UNIX - GNU
    ABI Version:                       0
    Type:                              EXEC (Executable file)
    Machine:                           AArch64
    Version:                           0x1
    Entry point address:               0x400640
    Start of program headers:          64 (bytes into file)
    Start of section headers:          640088 (bytes into file)
    Flags:                             0x0
    Size of this header:               64 (bytes)
    Size of program headers:           56 (bytes)
    Number of program headers:         6
    Size of section headers:           64 (bytes)
    Number of section headers:         32
    Section header string table index: 31
    ```
    Notice this time the ELF file type is `TYPE EXEC`.

    Now, lets look at the disassembled code for `main()` in the executable by running :

    ```bash
    00000000004007dc <main>:
    4007dc:       a9be7bfd        stp     x29, x30, [sp, #-32]!
    4007e0:       910003fd        mov     x29, sp
    4007e4:       528000a0        mov     w0, #0x5                        // #5
    4007e8:       b90017e0        str     w0, [sp, #20]
    4007ec:       52800280        mov     w0, #0x14                       // #20
    4007f0:       b9001be0        str     w0, [sp, #24]
    4007f4:       b9401be1        ldr     w1, [sp, #24]
    4007f8:       b94017e0        ldr     w0, [sp, #20]
    4007fc:       94000018        bl      40085c <my_add>
    400800:       b9001fe0        str     w0, [sp, #28]
    400804:       b9401fe3        ldr     w3, [sp, #28]
    400808:       b9401be2        ldr     w2, [sp, #24]
    40080c:       b94017e1        ldr     w1, [sp, #20]
    400810:       900002a0        adrp    x0, 454000 <__gcc_personality_v0+0x3c>
    400814:       9111a000        add     x0, x0, #0x468
    400818:       9400039a        bl      401680 <_IO_printf>
    40081c:       b9401fe0        ldr     w0, [sp, #28]
    400820:       97ffffe4        bl      4007b0 <isodd>
    400824:       39004fe0        strb    w0, [sp, #19]
    400828:       39404fe0        ldrb    w0, [sp, #19]
    40082c:       12000000        and     w0, w0, #0x1
    400830:       7100001f        cmp     w0, #0x0
    400834:       540000a0        b.eq    400848 <main+0x6c>  // b.none
    400838:       900002a0        adrp    x0, 454000 <__gcc_personality_v0+0x3c>
    40083c:       91122000        add     x0, x0, #0x488
    400840:       9400148c        bl      405a70 <_IO_puts>
    400844:       14000004        b       400854 <main+0x78>
    400848:       900002a0        adrp    x0, 454000 <__gcc_personality_v0+0x3c>
    40084c:       91128000        add     x0, x0, #0x4a0
    400850:       94001488        bl      405a70 <_IO_puts>
    400854:       52800000        mov     w0, #0x0                        // #0
    400858:       94000382        bl      401660 <exit>
    ```

    Filtering out the function calls, aka the `bl instructions`

    ```bash
    00000000004007dc <main>:
    4007fc:       94000018        bl      40085c <my_add>
    400818:       9400039a        bl      401680 <_IO_printf>
    400820:       97ffffe4        bl      4007b0 <isodd>
    400840:       9400148c        bl      405a70 <_IO_puts>
    400850:       94001488        bl      405a70 <_IO_puts>
    400858:       94000382        bl      401660 <exit>
    ```
    This time notice that all of the machine code for the `bl` instruction still starts with `100101` however the remaining zeros were replaced with offsets to where these functions are actually located.


