; "Hello World!" in 32 bit Linux NASM
; adapted from http://asm.sourceforge.net/intro/hello.html by Mark Loiseau
; referenced in http://blog.markloiseau.com/2012/04/hello-world-NASM-Linux
 
global _start            ; global entry point export for ld
 
section .text
_start:
 
    ; sys_write(stdout, message, length)
 
    mov eax, 4        ; sys_write syscall
    mov ebx, 1        ; stdout
    mov ecx, message    ; message address
    mov edx, length        ; message string length
    int 80h
 
    ; sys_exit(return_code)
 
    mov eax, 1        ; sys_exit syscall
    mov ebx, 0        ; return 0 (success)
    int 80h
 
section .data
    message: db 'Hello, world!',0x0A    ; message and newline
    length: equ $-message            ; NASM definition pseudo-instruction
