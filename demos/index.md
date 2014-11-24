---
title: nROP Examples
layout: base
---

# Gather gadgets

The simplest form of gadgets is two chains, as ROP chains, do the same thing. A sample binary containing [optiROP](https://media.blackhat.com/us-13/US-13-Quynh-OptiROP-Hunting-for-ROP-Gadgets-in-Style-Slides.pdf)

## Natural gadgets

Command to launch:

    % ./packer -t 4889d8c3 examples/opti | grep "equivalent! 3" -A3

Results

    Found equivalent! 3
       [X] add byte ptr [rax], al ; add byte ptr [rdi], cl ; add eax, 0xc3d88948 ; xchg rbx, rax ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] add byte ptr [rdi], cl ; add eax, 0xc3d88948 ; xchg rbx, rax ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] add eax, 0xc3d88948 ; xchg rbx, rax ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] fadd st0, st3 ; xchg rbx, rax ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] fadd st0, st3 ; xchg rcx, rbx ; xchg rcx, rax ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] fadd st0, st3 ; xor rax, rax ; not rax ; and rax, rbx ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] imul rax, rbx, 0x1 ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] mov rax, rbx ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] push rbx ; xor rax, rax ; pop rax ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] rcr byte ptr [rsi+0x48], 0x1 ; fadd st0, st3 ; xor rax, rax ; not rax ; and rax, rbx ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] ror byte ptr [rax+0x21], 0x1 ; fadd st0, st3 ; xchg rcx, rbx ; xchg rcx, rax ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] xchg rbx, rax ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] xchg rcx, rbx ; lea rax, ptr [rcx] ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] xchg rcx, rbx ; xchg rcx, rax ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] xor eax, eax ; add rax, rbx ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] xor eax, eax ; not rax ; and rax, rbx ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] xor rax, rax ; add rax, rbx ; ret  ;
       [X] mov rax, rbx ; ret  ;
    --
    Found equivalent! 3
       [X] xor rax, rax ; not rax ; and rax, rbx ; ret  ;
       [X] mov rax, rbx ; ret  ;


## Chained gadgets

Support coming :)
