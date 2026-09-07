section .text
bits 32
global syscall_isr
extern syscall_dispatch

syscall_isr:
    pusha
    push ds
    push es
    mov ax, 0x10
    mov ds, ax
    mov es, ax

    mov eax, [esp + 36]
    mov ebx, [esp + 24]
    mov ecx, [esp + 32]
    mov edx, [esp + 28]
    push edx
    push ecx
    push ebx
    push eax
    call syscall_dispatch
    add esp, 16
    mov [esp + 36], eax

    pop es
    pop ds
    popa
    iretd

section .note.GNU-stack,"",@progbits
