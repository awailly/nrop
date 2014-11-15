; 64-bit "Hello World!" in Linux NASM
 
global _start            ; global entry point export for ld
 
section .text
_start:
 
    ; sys_write(stdout, message, length)
    call mov1

mov1:
    int 3
    not rax
    and rax, rbx
    ret

section .data
    message: db 'Hello, world!',0x0A    ; message and newline
    length:    equ    $-message        ; NASM definition pseudo-instruction
