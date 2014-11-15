; 64-bit "Hello World!" in Linux NASM
 
global _start            ; global entry point export for ld
 
section .text
_start:
 
    ; sys_write(stdout, message, length)
    call do_stuff 
 
    ; sys_exit(return_code)
 
    mov    rax, 60        ; sys_exit
    mov    rdi, 0        ; return 0 (success)
    syscall

do_stuff:
    mov    eax, 0xc      ; sys_write
    sti
    mov    rdi, 1        ; stdout
    shl    eax, 0x4
    ret
 
section .data
    message: db 'Hello, world!',0x0A    ; message and newline
    length:    equ    $-message        ; NASM definition pseudo-instruction
