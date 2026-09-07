#ifndef TEST_OS_GRAPHICS_H
#define TEST_OS_GRAPHICS_H

#include <stdint.h>

struct framebuffer {
    uint8_t *address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
};

int graphics_init(uint32_t multiboot_magic, uint32_t multiboot_info);
const struct framebuffer *graphics_framebuffer(void);
int graphics_ready(void);
void graphics_clear(uint32_t pixel);
void graphics_putpixel(uint32_t x, uint32_t y, uint32_t pixel);

#endif
