#include "syscall.h"
#include "process.h"
#include "desktop.h"
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

static uint8_t user_row = 12;
static uint8_t user_column = 0;

static void user_console_putchar(char c) {
    if (c == '\n' || user_column >= VGA_WIDTH) {
        user_column = 0;
        ++user_row;
        if (user_row >= VGA_HEIGHT) user_row = 12;
        if (c == '\n') return;
    }
    VGA_MEMORY[user_row * VGA_WIDTH + user_column] = ((uint16_t)0x0A << 8) | (uint8_t)c;
    ++user_column;
}

static void user_console_write(const char *text) {
    if (!text) return;
    for (uint32_t i = 0; i < 256 && text[i]; ++i) user_console_putchar(text[i]);
}

uint32_t syscall_dispatch(uint32_t number, uint32_t arg0, uint32_t arg1, uint32_t arg2) {
    (void)arg1;
    (void)arg2;

    switch (number) {
        case SYS_EXIT:
            return 0;
        case SYS_WRITE:
            user_console_write((const char*)arg0);
            return 0;
        case SYS_YIELD:
            /* The first userspace process doubles as the desktop event loop. */
            desktop_update();
            __asm__ volatile ("pause");
            return 0;
        case SYS_GETPID:
            return process_count() ? process_table()[0].pid : 0;
        default:
            return (uint32_t)-1;
    }
}
