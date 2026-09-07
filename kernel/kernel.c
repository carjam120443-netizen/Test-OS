#include <stdint.h>

static volatile uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static uint8_t row = 0;
static uint8_t column = 0;
static uint8_t color = 0x07;

static void terminal_clear(void) {
    for (uint32_t i = 0; i < 80 * 25; ++i) {
        VGA_MEMORY[i] = ((uint16_t)color << 8) | ' ';
    }
    row = 0;
    column = 0;
}

static void terminal_putchar(char c) {
    if (c == '\n') {
        column = 0;
        ++row;
        return;
    }

    VGA_MEMORY[row * 80 + column] = ((uint16_t)color << 8) | (uint8_t)c;
    if (++column >= 80) {
        column = 0;
        ++row;
    }
}

static void terminal_write(const char* text) {
    while (*text) {
        terminal_putchar(*text++);
    }
}

void kernel_main(void) {
    terminal_clear();
    terminal_write("===========================\n");
    terminal_write("       TEST-OS KERNEL      \n");
    terminal_write("===========================\n\n");
    terminal_write("Test-OS has booted successfully!\n");
    terminal_write("Kernel: 0.1.0\n");
    terminal_write("Architecture: x86\n");
    terminal_write("Status: ONLINE\n\n");
    terminal_write("testos> ");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
