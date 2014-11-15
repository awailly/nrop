global _start

_start:
    mov dword [rax + 1], 10
    mov rbx, 20
    xor rax, rbx
    push rax
    ret
