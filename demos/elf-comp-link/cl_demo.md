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

5.  Remember the `UDF` undefined symbols from the object file, lets look at what happened to them.  Running `readelf -s ./cl_demo | more` shows that we now have a lot of symbols, 3059 to be exact.  Many of these come from the C runtime library.  To filter out the ones that we want we use `grep`.  For example: `readelf -s ./cl_demo | grep "\bmain"` will look for the `main` function symbol. The current situation is shown below:

    ```bash
    Symbol table '.symtab' contains 3059 entries:
     Num:    Value          Size Type    Bind   Vis      Ndx Name
       6: 00000000004002c0     0 SECTION LOCAL  DEFAULT    6 .text
    2179: 00000000004007b0    44 FUNC    GLOBAL DEFAULT    6 isodd
    2624: 00000000004007dc   128 FUNC    GLOBAL DEFAULT    6 main
    2636: 000000000040085c    40 FUNC    GLOBAL DEFAULT    6 my_add
    1938: 0000000000401680   196 FUNC    GLOBAL DEFAULT    6 printf
    2133: 0000000000405a70   584 FUNC    WEAK   DEFAULT    6 puts
    2875: 0000000000401660    28 FUNC    GLOBAL HIDDEN     6 exit
    ```

    Take a look at the value for main, its `0x00000000004007dc`. Now, lets look at the disassembled code for `main()` in the executable by running `objdump -d --disassemble=main ./cl_demo`.  **NOTICE that the first instruction in main is at location `0x00000000004007dc`, the same value in the symbol table.**

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

6. Next, lets see what the linker did to the function calls.  Filtering out the function calls, aka the `bl`:

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

    Lets dig into a few example. Lets look at the call to the `isodd()` function.  From above:

    ```
    400820:       97ffffe4        bl      4007b0 <isodd>
    ```
    Now lets disassemble `isodd()` using `objdump -d --disassemble=isodd ./cl_demo`:

    ```bash
    00000000004007b0 <isodd>:
    4007b0:       d10043ff        sub     sp, sp, #0x10
    4007b4:       b9000fe0        str     w0, [sp, #12]
    4007b8:       b9400fe0        ldr     w0, [sp, #12]
    4007bc:       12000000        and     w0, w0, #0x1
    4007c0:       7100001f        cmp     w0, #0x0
    4007c4:       54000060        b.eq    4007d0 <isodd+0x20>  // b.none
    4007c8:       52800020        mov     w0, #0x1                        // #1
    4007cc:       14000002        b       4007d4 <isodd+0x24>
    4007d0:       52800000        mov     w0, #0x0                        // #0
    4007d4:       910043ff        add     sp, sp, #0x10
    4007d8:       d65f03c0        ret
    ```
    Notice the `isodd()` function starts at position `0x4007b0`. And in `main()` the `bl` instrucion to call `isodd()`

    ```
    400820:       97ffffe4        bl      4007b0 <isodd>
    ```

    Thus the linker replaced the address of the code generated in the object file `bl 0` with `bl 4007b0` which is the address of the implementation.  If we do this for other functions like `my_add()` via `objdump -d --disassemble=my_add ./cl_demo` we will see the same behavior, the `my_add()` function was resolved by the linker with its code starting at `0x40085c`.  We will see this for all of the functions, even the ones in the C runtime library since we dynamically linked this version of the executable.

7.  So what are all of these addresses? They are relitive addresses based on assuming the operating system loads the program starting at memory address 0.  More on this later.  But for now, look at the ELF header for the executable again by running `readelf -h ./cl_demo`.  This time look at the `Entry point address:` field:

    ```
    Entry point address:               0x400640
    ```

    What exactly is located there?  Run `objdump --disassemble=_start ./cl_demo`. Notice the ELF file instructs the program to bootstrap by running the `_start()` function. This function does a bunch of things to setup the C runtime library.  The last thing that it does is to branch to `main()` via `b  4007dc`.  From above that is exactly where main is located in this executable. 

    ```
    0000000000400640 <_start>:
    400640:       d503201f        nop
    400644:       d280001d        mov     x29, #0x0                       // #0
    400648:       d280001e        mov     x30, #0x0                       // #0
    40064c:       aa0003e5        mov     x5, x0
    400650:       f94003e1        ldr     x1, [sp]
    400654:       910023e2        add     x2, sp, #0x8
    400658:       910003e6        mov     x6, sp
    40065c:       90000000        adrp    x0, 400000 <__ehdr_start>
    400660:       9119d000        add     x0, x0, #0x674
    400664:       d2800003        mov     x3, #0x0                        // #0
    400668:       d2800004        mov     x4, #0x0                        // #0
    40066c:       940000c2        bl      400974 <__libc_start_main>
    400670:       97ffff14        bl      4002c0 <abort>

    0000000000400674 <__wrap_main>:
    400674:       d503201f        nop
    400678:       14000059        b       4007dc <main>
    ```

    Getting back to all of these addresses.  Linux uses a position independent code model.  The executable generates all addresses assuming the program is loaded at memory location 0.  If that was the case the `_start()` function to get things running would be positioned in memory location `0x400640`. 

    Linux uses a special security feature called Address Space Layout Randomization (ASLR).  You can see if this is enabled by `cat /proc/sys/kernel/randomize_va_space`.  If the value returned is `0` ASLR is disabled, `2` is fully enabled, and `1` is partially enabled. 

#### Dynamic Linking Deep Dive

Next, we recompile for dynamic linking which is the default.  

```bash
touch *.c
make
```
We next look at the symbol table of the executable:

```bash
readelf -s ./cl_demo | grep "\bUND"
```
We now see there are a number of **undefined (UND)** references.  

```bash
bsm23@code:~/SysProg-Class/demos/elf-comp-link$ readelf -s ./cl_demo | grep "\bUND"
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     3: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND exit@GLIBC_2.17 (2)
     4: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND _[...]@GLIBC_2.34 (3)
     5: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterT[...]
     6: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND _[...]@GLIBC_2.17 (2)
     7: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
     8: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND abort@GLIBC_2.17 (2)
     9: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@GLIBC_2.17 (2)
    10: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMC[...]
    11: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.17 (2)
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
    74: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND exit@GLIBC_2.17
    75: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_mai[...]
    76: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterT[...]
    79: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@G[...]
    86: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
    88: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND abort@GLIBC_2.17
    90: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@GLIBC_2.17
    98: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMC[...]
    99: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf@GLIBC_2.17
```
Next we can look at a special section in the ELF file called the program linkage table (PLT):

```bash
bsm23@code:~/SysProg-Class/demos/elf-comp-link$ objdump -d -j .plt ./cl_demo

./cl_demo:     file format elf64-littleaarch64


Disassembly of section .plt:

0000000000000640 <.plt>:
 640:   a9bf7bf0        stp     x16, x30, [sp, #-16]!
 644:   f00000f0        adrp    x16, 1f000 <__FRAME_END__+0x1e588>
 648:   f947ca11        ldr     x17, [x16, #3984]
 64c:   913e4210        add     x16, x16, #0xf90
 650:   d61f0220        br      x17
 654:   d503201f        nop
 658:   d503201f        nop
 65c:   d503201f        nop

0000000000000660 <exit@plt>:
 660:   f00000f0        adrp    x16, 1f000 <__FRAME_END__+0x1e588>
 664:   f947ce11        ldr     x17, [x16, #3992]
 668:   913e6210        add     x16, x16, #0xf98
 66c:   d61f0220        br      x17

0000000000000670 <__libc_start_main@plt>:
 670:   f00000f0        adrp    x16, 1f000 <__FRAME_END__+0x1e588>
 674:   f947d211        ldr     x17, [x16, #4000]
 678:   913e8210        add     x16, x16, #0xfa0
 67c:   d61f0220        br      x17

0000000000000680 <__cxa_finalize@plt>:
 680:   f00000f0        adrp    x16, 1f000 <__FRAME_END__+0x1e588>
 684:   f947d611        ldr     x17, [x16, #4008]
 688:   913ea210        add     x16, x16, #0xfa8
 68c:   d61f0220        br      x17

0000000000000690 <__gmon_start__@plt>:
 690:   f00000f0        adrp    x16, 1f000 <__FRAME_END__+0x1e588>
 694:   f947da11        ldr     x17, [x16, #4016]
 698:   913ec210        add     x16, x16, #0xfb0
 69c:   d61f0220        br      x17

00000000000006a0 <abort@plt>:
 6a0:   f00000f0        adrp    x16, 1f000 <__FRAME_END__+0x1e588>
 6a4:   f947de11        ldr     x17, [x16, #4024]
 6a8:   913ee210        add     x16, x16, #0xfb8
 6ac:   d61f0220        br      x17

00000000000006b0 <puts@plt>:
 6b0:   f00000f0        adrp    x16, 1f000 <__FRAME_END__+0x1e588>
 6b4:   f947e211        ldr     x17, [x16, #4032]
 6b8:   913f0210        add     x16, x16, #0xfc0
 6bc:   d61f0220        br      x17

00000000000006c0 <printf@plt>:
 6c0:   f00000f0        adrp    x16, 1f000 <__FRAME_END__+0x1e588>
 6c4:   f947e611        ldr     x17, [x16, #4040]
 6c8:   913f2210        add     x16, x16, #0xfc8
 6cc:   d61f0220        br      x17
 ```
We see for each undefined function there is a `function_name@plt` entry.  

The next thing we want to look at is the Global Offset Table (GOT).  Step 1 is to look at the relocations:

```bash
bsm23@code:~/SysProg-Class/demos/elf-comp-link$ readelf -r ./cl_demo

Relocation section '.rela.dyn' at offset 0x4c0 contains 8 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
00000001fd80  000000000403 R_AARCH64_RELATIV                    810
00000001fd88  000000000403 R_AARCH64_RELATIV                    7c0
00000001fff0  000000000403 R_AARCH64_RELATIV                    844
000000020008  000000000403 R_AARCH64_RELATIV                    20008
00000001ffd8  000500000401 R_AARCH64_GLOB_DA 0000000000000000 _ITM_deregisterTM[...] + 0
00000001ffe0  000600000401 R_AARCH64_GLOB_DA 0000000000000000 __cxa_finalize@GLIBC_2.17 + 0
00000001ffe8  000700000401 R_AARCH64_GLOB_DA 0000000000000000 __gmon_start__ + 0
00000001fff8  000a00000401 R_AARCH64_GLOB_DA 0000000000000000 _ITM_registerTMCl[...] + 0

Relocation section '.rela.plt' at offset 0x580 contains 7 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
00000001ff98  000300000402 R_AARCH64_JUMP_SL 0000000000000000 exit@GLIBC_2.17 + 0
00000001ffa0  000400000402 R_AARCH64_JUMP_SL 0000000000000000 __libc_start_main@GLIBC_2.34 + 0
00000001ffa8  000600000402 R_AARCH64_JUMP_SL 0000000000000000 __cxa_finalize@GLIBC_2.17 + 0
00000001ffb0  000700000402 R_AARCH64_JUMP_SL 0000000000000000 __gmon_start__ + 0
00000001ffb8  000800000402 R_AARCH64_JUMP_SL 0000000000000000 abort@GLIBC_2.17 + 0
00000001ffc0  000900000402 R_AARCH64_JUMP_SL 0000000000000000 puts@GLIBC_2.17 + 0
00000001ffc8  000b00000402 R_AARCH64_JUMP_SL 0000000000000000 printf@GLIBC_2.17 + 0
```
Lets look at the line for `printf`:

```bash
00000001ffc8  000b00000402 R_AARCH64_JUMP_SL 0000000000000000 printf@GLIBC_2.17 + 0
```
Notice the address `0x1ffc8`.  Now lets go back to the disassembly for `printf@plt` from above:

```bash
00000000000006c0 <printf@plt>:
 6c0:   f00000f0        adrp    x16, 1f000 <__FRAME_END__+0x1e588>
 6c4:   f947e611        ldr     x17, [x16, #4040]
 6c8:   913f2210        add     x16, x16, #0xfc8
 6cc:   d61f0220        br      x17
```
The above references the inital starting point of the `GOT` to be at address `1f000`.  The next two lines load up the `X16` and `X17` registers with `0x1000 + 0xfc8 = 0x1fc8`.  Lets remember that value.

Now lets go to the `GOT` in the ELF file:

```bash
Hex dump of section '.got':
 NOTE: This section has relocations against it, but these have NOT been applied to this dump.
  0x0001ff80 00000000 00000000 00000000 00000000 ................
  0x0001ff90 00000000 00000000 40060000 00000000 ........@.......
  0x0001ffa0 40060000 00000000 40060000 00000000 @.......@.......
  0x0001ffb0 40060000 00000000 40060000 00000000 @.......@.......
  0x0001ffc0 40060000 00000000 40060000 00000000 @.......@.......
  0x0001ffd0 90fd0100 00000000 00000000 00000000 ................
  0x0001ffe0 00000000 00000000 00000000 00000000 ................
  0x0001fff0 44080000 00000000 00000000 00000000 D...............
  ```
  We see several things repeating, but one thing of interest is that at address `0x1ffc8` is the memory address `0x40060000 00000000`





