ENTRY(_start)

SECTIONS
{
    . = 1M;

    .text : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }

    .data : ALIGN(4K)
    {
        *(.data)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* User programs live in the identity-mapped 4 MiB region. */
    . = 4M;
    .user : ALIGN(4K)
    {
        __user_start = .;
        KEEP(*(.user.text))
        KEEP(*(.user.rodata))
        KEEP(*(.user.data))
        KEEP(*(.user.bss))
        __user_end = .;
    }
}
