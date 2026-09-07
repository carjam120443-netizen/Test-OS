; Test-OS Multiboot entry
section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

section .text
bits 32
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    ; Multiboot supplies EAX=magic and EBX=info structure.
    push ebx
    push eax
    call kernel_main
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .note.GNU-stack,"",@progbits
