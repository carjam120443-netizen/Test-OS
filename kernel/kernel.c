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
#include "compositor.h"
#include "desktop.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

/* Keep the graphical shell testable while the ring-3 path is being debugged. */
#define TESTOS_BOOT_TO_DESKTOP 1

extern const uint8_t __initramfs_start[];
extern const uint8_t __initramfs_end[];
static uint8_t row, column;
static const uint8_t color = 0x07;

static void terminal_clear(void) { for (uint32_t i=0;i<VGA_WIDTH*VGA_HEIGHT;++i) VGA_MEMORY[i]=((uint16_t)color<<8)|' '; row=column=0; }
static void terminal_putchar(char c) { if(c=='\n'||column>=VGA_WIDTH){column=0;++row;} if(row>=VGA_HEIGHT)row=0; if(c!='\n')VGA_MEMORY[row*VGA_WIDTH+column++]=((uint16_t)color<<8)|(uint8_t)c; }
static void terminal_write(const char *s) { while(*s) terminal_putchar(*s++); }

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info) {
    terminal_clear();
    terminal_write("===========================\nTEST-OS KERNEL 0.5.0\n===========================\n\nArchitecture: x86 / i386\n");
    terminal_write("Graphics: ");
    terminal_write(graphics_init(multiboot_magic, multiboot_info) ? "FRAMEBUFFER READY\n" : "VGA FALLBACK\n");
    compositor_init();
    terminal_write("Compositor: READY (window/surface registry)\n");
    terminal_write("Desktop: initializing native graphical shell...\n");
    desktop_init();
    terminal_write("Desktop: READY (framebuffer + panel + starter window)\n");

#if TESTOS_BOOT_TO_DESKTOP
    terminal_write("Boot mode: DESKTOP SAFE MODE (ring 3 temporarily disabled)\n");
    terminal_write("Desktop: entering kernel event loop...\n");
    for (;;) {
        desktop_update();
    }
#endif

    terminal_write("Network: initializing E1000...\n");
    net_init();
    gdt_init(); terminal_write("GDT: READY\n");
    terminal_write("Paging: "); terminal_write(paging_init() ? "BOOTSTRAP READY\n" : "FAILED\n");
    tss_init(); terminal_write("TSS: READY\n");
    idt_init(); terminal_write("IDT: READY (int 0x80)\n");

    uint32_t initramfs_size=(uint32_t)(__initramfs_end-__initramfs_start);
    terminal_write("Filesystem: "); terminal_write(fs_init(__initramfs_start,initramfs_size)?"ramfs READY\n":"FAILED\n");
    const struct fs_file *init_file=fs_open("/bin/init");
    struct elf_load_result image;
    int loaded=init_file&&elf_load(fs_data(init_file),fs_size(init_file),&image);
    terminal_write("ELF loader: "); terminal_write(loaded?"ELF32 LOADED\n":"FAILED\n");
    if(!loaded) for(;;)__asm__ volatile("hlt");

    uint32_t user_pd=paging_create_user_space();
    int init_pid=process_create("init",image.entry,image.image_start,image.image_end,user_pd);
    terminal_write("Process: "); terminal_write(init_pid>0?"PID 1 + address space READY\n":"FAILED\n");
    if(init_pid<=0||!paging_switch(user_pd)) for(;;)__asm__ volatile("hlt");
    terminal_write("Exec path: kernel -> process -> virtual address space -> ELF -> ring 3\n");
    terminal_write("libc + ramfs + framebuffer + compositor + desktop: ONLINE\n");
    terminal_write("Launching /bin/init...\n");
    enter_user_mode(image.entry, TESTOS_USER_STACK_TOP);
    for(;;)__asm__ volatile("hlt");
}
