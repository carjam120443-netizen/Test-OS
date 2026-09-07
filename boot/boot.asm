; Test-OS boot entry
; Multiboot-compatible 32-bit entry point. The kernel will transition
; to the architecture-specific environment as the project grows.

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
    call kernel_main
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
