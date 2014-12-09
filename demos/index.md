---
title: nROP Examples
layout: base
---

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

## Chained gadgets

Support coming :)

### PC1

This category of gadgets is close to PN1, setting register to another, but with the help of gadget chaining.

### PC2

This category of gadgets is close to PN2, setting register to another, but with the help of gadget chaining.

### PC3

This category of gadgets is close to PN3, setting register to another, but with the help of gadget chaining.

