#include "graphics.h"

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002u
#define MBI_FLAG_FRAMEBUFFER (1u << 12)

struct multiboot_info { uint32_t flags; uint32_t mem_lower, mem_upper, boot_device, cmdline, mods_count, mods_addr; uint8_t syms[16]; uint32_t mmap_length, mmap_addr; uint32_t drives_length, drives_addr, config_table, boot_loader_name, apm_table, vbe_control_info, vbe_mode_info; uint16_t vbe_mode, vbe_interface_seg, vbe_interface_off, vbe_interface_len; uint64_t framebuffer_addr; uint32_t framebuffer_pitch, framebuffer_width, framebuffer_height; uint8_t framebuffer_bpp, framebuffer_type; uint8_t color_info[6]; } __attribute__((packed));

static struct framebuffer fb;
static int ready;

int graphics_init(uint32_t magic, uint32_t info_addr) {
    ready = 0;
    fb.address = 0; fb.width = fb.height = fb.pitch = fb.bpp = 0;
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC || !info_addr) return 0;
    const struct multiboot_info *mbi = (const struct multiboot_info *)(uintptr_t)info_addr;
    if (!(mbi->flags & MBI_FLAG_FRAMEBUFFER) || mbi->framebuffer_type != 1 || mbi->framebuffer_bpp < 24) return 0;
    if (mbi->framebuffer_addr > 0xFFFFFFFFull) return 0;
    fb.address = (uint8_t *)(uintptr_t)mbi->framebuffer_addr;
    fb.width = mbi->framebuffer_width;
    fb.height = mbi->framebuffer_height;
    fb.pitch = mbi->framebuffer_pitch;
    fb.bpp = mbi->framebuffer_bpp;
    ready = fb.address && fb.width && fb.height && fb.pitch;
    return ready;
}

const struct framebuffer *graphics_framebuffer(void) { return &fb; }
int graphics_ready(void) { return ready; }

void graphics_clear(uint32_t pixel) {
    if (!ready) return;
    for (uint32_t y = 0; y < fb.height; ++y)
        for (uint32_t x = 0; x < fb.width; ++x) graphics_putpixel(x, y, pixel);
}

void graphics_putpixel(uint32_t x, uint32_t y, uint32_t pixel) {
    if (!ready || x >= fb.width || y >= fb.height) return;
    volatile uint32_t *p = (volatile uint32_t *)(fb.address + y * fb.pitch + x * (fb.bpp / 8));
    *p = pixel;
}
