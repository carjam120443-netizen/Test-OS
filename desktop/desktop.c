#include <stdint.h>
#include "desktop.h"
#include "graphics.h"
#include "compositor.h"

static void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t pixel) {
    const struct framebuffer *fb = graphics_framebuffer();
    if (!graphics_ready() || x >= fb->width || y >= fb->height) return;
    if (w > fb->width - x) w = fb->width - x;
    if (h > fb->height - y) h = fb->height - y;
    for (uint32_t yy = y; yy < y + h; ++yy)
        for (uint32_t xx = x; xx < x + w; ++xx)
            graphics_putpixel(xx, yy, pixel);
}

static void draw_border(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t pixel) {
    if (w < 2 || h < 2) return;
    fill_rect(x, y, w, 2, pixel);
    fill_rect(x, y + h - 2, w, 2, pixel);
    fill_rect(x, y, 2, h, pixel);
    fill_rect(x + w - 2, y, 2, h, pixel);
}

static void draw_x(uint32_t x, uint32_t y, uint32_t pixel) {
    for (uint32_t i = 0; i < 12; ++i) {
        graphics_putpixel(x + i, y + i, pixel);
        graphics_putpixel(x + 11 - i, y + i, pixel);
        graphics_putpixel(x + i, y + i + 1, pixel);
        graphics_putpixel(x + 11 - i, y + i + 1, pixel);
    }
}

void desktop_draw(void) {
    if (!graphics_ready()) return;

    const struct framebuffer *fb = graphics_framebuffer();
    const uint32_t bg = 0x001B2430;
    const uint32_t panel = 0x00232F3D;
    const uint32_t window = 0x00E7EDF2;
    const uint32_t title = 0x003B82F6;
    const uint32_t text = 0x00131A21;
    const uint32_t accent = 0x0060A5FA;
    const uint32_t white = 0x00FFFFFF;

    graphics_clear(bg);

    uint32_t panel_h = fb->height >= 40 ? 40 : fb->height;
    fill_rect(0, fb->height - panel_h, fb->width, panel_h, panel);

    uint32_t button_w = fb->width >= 220 ? 120 : fb->width / 2;
    fill_rect(8, fb->height - panel_h + 6, button_w, panel_h - 12, title);

    if (fb->width >= 300 && fb->height >= 180) {
        uint32_t ww = fb->width * 3 / 5;
        uint32_t wh = fb->height * 3 / 5;
        uint32_t wx = (fb->width - ww) / 2;
        uint32_t wy = (fb->height - panel_h - wh) / 2;
        if (ww < 180) ww = 180;
        if (wh < 100) wh = 100;

        fill_rect(wx, wy, ww, wh, window);
        fill_rect(wx, wy, ww, 28, title);
        draw_border(wx, wy, ww, wh, accent);
        draw_x(wx + ww - 24, wy + 7, white);

        /* Simple geometric "text" blocks keep the first desktop freestanding. */
        fill_rect(wx + 24, wy + 52, ww > 80 ? ww - 48 : 20, 6, text);
        fill_rect(wx + 24, wy + 68, ww > 120 ? ww - 72 : 20, 5, text);
        fill_rect(wx + 24, wy + 83, ww > 150 ? ww - 100 : 20, 5, text);
        fill_rect(wx + 24, wy + 108, ww > 100 ? 96 : 20, 28, accent);
    }
}

void desktop_init(void) {
    compositor_init();
    (void)compositor_create_window(0, 0, 320, 200);
    desktop_draw();
}
