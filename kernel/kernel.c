#include <stdint.h>
#include "net.h"
#include "process.h"
#include "syscall.h"
#include "gdt.h"
#include "tss.h"
#include "paging.h"
#include "idt.h"
#include "fs.h"
#include "elf.h"
#include "graphics.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

extern const uint8_t __initramfs_start[];
extern const uint8_t __initramfs_end[];

static uint8_t row;
static uint8_t column;
static const uint8_t color = 0x07;

static void terminal_clear(void) {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i)
        VGA_MEMORY[i] = ((uint16_t)color << 8) | ' ';
    row = column = 0;
}

static void terminal_putchar(char c) {
    if (c == '\n' || column >= VGA_WIDTH) { column = 0; ++row; }
    if (row >= VGA_HEIGHT) row = 0;
    if (c != '\n') VGA_MEMORY[row * VGA_WIDTH + column++] = ((uint16_t)color << 8) | (uint8_t)c;
}

static void terminal_write(const char *s) { while (*s) terminal_putchar(*s++); }

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info) {
    terminal_clear();
    terminal_write("===========================\n");
    terminal_write("       TEST-OS KERNEL      \n");
    terminal_write("===========================\n\n");
    terminal_write("Test-OS has booted successfully!\n");
    terminal_write("Kernel: 0.4.0\n");
    terminal_write("Architecture: x86 / i386\n");

    terminal_write("Graphics: ");
    terminal_write(graphics_init(multiboot_magic, multiboot_info) ? "MULTIBOOT FRAMEBUFFER READY\n" : "VGA FALLBACK\n");

    terminal_write("Network: initializing E1000...\n");
    net_init();

    terminal_write("GDT: ");
    gdt_init();
    terminal_write("READY\n");
    terminal_write("Paging: ");
    terminal_write(paging_init() ? "16 MiB bootstrap address space READY\n" : "FAILED\n");
    terminal_write("TSS: ");
    tss_init();
    terminal_write("READY\n");
    terminal_write("IDT: ");
    idt_init();
    terminal_write("READY (int 0x80)\n");

    uint32_t initramfs_size = (uint32_t)(__initramfs_end - __initramfs_start);
    terminal_write("Filesystem: ");
    terminal_write(fs_init(__initramfs_start, initramfs_size) ? "ramfs READY\n" : "FAILED\n");

    const struct fs_file *init_file = fs_open("/bin/init");
    struct elf_load_result image;
    terminal_write("ELF loader: ");
    terminal_write(init_file && elf_load(fs_data(init_file), fs_size(init_file), &image) ? "ELF32 loaded at 0x00400000\n" : "FAILED\n");
    if (!init_file || !elf_load(fs_data(init_file), fs_size(init_file), &image)) {
        terminal_write("Unable to load userspace init. Halting.\n");
        for (;;) __asm__ volatile ("hlt");
    }

    int init_pid = process_create("init", image.entry, image.image_start, image.image_end);
    terminal_write("Process: ");
    terminal_write(init_pid > 0 ? "PID 1 created\n" : "FAILED\n");
    terminal_write("Userspace: kernel -> process -> virtual address space -> ELF -> ring 3\n");
    terminal_write("libc + ramfs + framebuffer foundations are online.\n\n");
    terminal_write("Launching /bin/init...\n");
    enter_user_mode(image.entry, 0x00800000);

    for (;;) __asm__ volatile ("hlt");
}
