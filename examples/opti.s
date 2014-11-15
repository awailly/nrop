; 64-bit "Hello World!" in Linux NASM
 
global _start            ; global entry point export for ld
 
section .text
_start:
 
    ; sys_write(stdout, message, length)
    call mov1
    call mov2
    call mov3
    call mov4
    call mov5
    call mov6
    call mov7
    
    call edx0

    call eaxedx

    call nat1
    call nat2
    call nat3

    call cha1
    call cha2
    call cha3
    call cha4
    call cha5
    call cha6
 
    ; sys_exit(return_code)
 
    mov    rax, 60        ; sys_exit
    mov    rdi, 0        ; return 0 (success)
    syscall

mov1:
    mov rax, rbx
    ret

mov2:
    xchg rbx, rax
    ret
 
mov3:
    imul rax, rbx, 1
    ret

mov4:
    xor rax, rax
    pop rsi
    add rax, rbx
    ret

mov5:
    xor rax, rax
    not rax
    and rax, rbx
    ret

mov6:
    xchg rbx, rcx
    xchg rcx, rax
    ret

mov7:
    push rbx
    xor rax, rax
    pop rax
    ret

edx0:
    xor rax, rax
    xchg rdx, rax
    ret

eaxedx:
    mov rsi, 0xfffffffffffffff8
    lea rax, [rsi + rdx + 8]
    ret

nat1:
    xor rax, rax
    add rax, rbx
    ret

nat2:
    or rbx, 0xffffffffffffffff
    xchg rax, rbx
    ret

nat3:
    push 0x1234
    pop rax
    inc rbx
    ret

cha1:
    mov rbx, rdx
    xchg rbx, rcx
    lea rax, [rcx]
    ret

cha2:
    imul rcx, [rsi], 0x0
    xchg rcx, rax
    ret

cha3:
    push 0x1234
    pop rbp
    xchg rbp, rax
    ret

cha4:
    imul rcx, [rsi], 0
    add rcx, rax
    ret

cha5:
    push 0xffffffffffffedcc
    pop rdx
    xor rax, rax
    sub rax, rdx
    ret

cha6:
    xor rax, rax
    not rax
    and rax, rdx
    add rax, rbx
    ret

section .data
    message: db 'Hello, world!',0x0A    ; message and newline
    length:    equ    $-message        ; NASM definition pseudo-instruction
