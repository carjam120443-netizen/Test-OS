#include <stdint.h>
#include "desktop.h"
#include "graphics.h"
#include "compositor.h"
#include "input.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t *)0xB8000)

static int starter_window;
static int terminal_window;
static uint8_t previous_buttons;
static uint8_t launcher_open;
static uint8_t dragging;
static int32_t drag_dx, drag_dy;

static void vga_clear(void) {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i)
        VGA_MEMORY[i] = ((uint16_t)0x07 << 8) | ' ';
}

static void vga_put(uint32_t x, uint32_t y, char c, uint8_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    VGA_MEMORY[y * VGA_WIDTH + x] = ((uint16_t)color << 8) | (uint8_t)c;
}

static void vga_text(uint32_t x, uint32_t y, const char *s, uint8_t color) {
    while (*s && x < VGA_WIDTH) {
        if (*s == '\n') { ++y; x = 0; }
        else vga_put(x++, y, *s, color);
        if (y >= VGA_HEIGHT) return;
        ++s;
    }
}

static void desktop_draw_vga(void) {
    vga_clear();
    vga_text(0, 0, "============================================================", 0x1F);
    vga_text(0, 1, "                    TEST-OS DESKTOP", 0x1F);
    vga_text(0, 2, "============================================================", 0x1F);
    vga_text(0, 4, "[ START ]", 0x1E);
    vga_text(0, 6, "+----------------------------------------------------------+", 0x17);
    vga_text(0, 7, "| TEST-OS DESKTOP                                          X |", 0x1F);
    vga_text(0, 8, "+----------------------------------------------------------+", 0x17);
    vga_text(0, 10, "| Graphics framebuffer unavailable.                         |", 0x07);
    vga_text(0, 11, "| Running the VGA text-mode desktop fallback.              |", 0x07);
    vga_text(0, 13, "| The kernel is alive and the desktop event loop is ready. |", 0x07);
    vga_text(0, 15, "+----------------------------------------------------------+", 0x17);
    vga_text(0, 17, "TERMINAL", 0x0F);
    vga_text(0, 18, "Type a command once userspace input is connected.", 0x07);
    vga_text(0, 22, "TEST-OS SAFE DESKTOP MODE", 0x1F);
    vga_text(0, 23, "Framebuffer: unavailable | VGA fallback: ACTIVE", 0x07);
}

static void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t pixel) {
    const struct framebuffer *fb = graphics_framebuffer();
    if (!graphics_ready() || x >= fb->width || y >= fb->height) return;
    if (w > fb->width - x) w = fb->width - x;
    if (h > fb->height - y) h = fb->height - y;
    for (uint32_t yy = y; yy < y + h; ++yy)
        for (uint32_t xx = x; xx < x + w; ++xx) graphics_putpixel(xx, yy, pixel);
}

static void draw_border(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t pixel) {
    if (w < 2 || h < 2) return;
    fill_rect(x, y, w, 2, pixel); fill_rect(x, y + h - 2, w, 2, pixel);
    fill_rect(x, y, 2, h, pixel); fill_rect(x + w - 2, y, 2, h, pixel);
}

static const uint8_t font[36][7] = {
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
 {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,14},{17,17,17,31,17,17,17},
 {31,4,4,4,4,4,31},{7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
 {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
 {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
 {2,6,10,18,31,2,2},{31,16,16,30,1,1,30},{14,17,16,30,17,17,14},{31,1,2,4,8,8,8},
 {14,17,1,6,1,17,14},{30,17,17,30,17,17,30}
};

static int font_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return 26 + c - '0';
    return -1;
}

static void draw_char(uint32_t x, uint32_t y, char c, uint32_t pixel, uint32_t scale) {
    int index = font_index(c);
    if (index < 0) return;
    for (uint32_t row = 0; row < 7; ++row)
        for (uint32_t col = 0; col < 5; ++col)
            if (font[index][row] & (1u << (4 - col))) fill_rect(x + col * scale, y + row * scale, scale, scale, pixel);
}

static void draw_text(uint32_t x, uint32_t y, const char *s, uint32_t pixel, uint32_t scale) {
    while (*s) { if (*s == ' ') x += 6 * scale; else { draw_char(x, y, *s, pixel, scale); x += 6 * scale; } ++s; }
}

static void draw_cursor(int32_t x, int32_t y, uint32_t pixel) {
    for (uint32_t i = 0; i < 14; ++i) {
        graphics_putpixel((uint32_t)x, (uint32_t)(y + i), pixel);
        graphics_putpixel((uint32_t)(x + i / 2), (uint32_t)(y + i), pixel);
    }
    for (uint32_t i = 0; i < 6; ++i) graphics_putpixel((uint32_t)(x + i), (uint32_t)(y + i), pixel);
}

static int inside(int32_t x, int32_t y, uint32_t rx, uint32_t ry, uint32_t rw, uint32_t rh) {
    return x >= (int32_t)rx && y >= (int32_t)ry && x < (int32_t)(rx + rw) && y < (int32_t)(ry + rh);
}

static void draw_window(const struct window *w, const char *title, uint32_t title_pixel, uint32_t body_pixel, uint32_t text_pixel) {
    if (!w || !(w->flags & 1)) return;
    fill_rect(w->x, w->y, w->width, w->height, body_pixel);
    fill_rect(w->x, w->y, w->width, 30, title_pixel);
    draw_border(w->x, w->y, w->width, w->height, 0x0060A5FA);
    draw_text(w->x + 10, w->y + 8, title, 0x00FFFFFF, 2);
    fill_rect(w->x + w->width - 25, w->y + 7, 15, 15, 0x00D94A4A);
    draw_text(w->x + 14, w->y + 48, "TEST OS", text_pixel, 2);
}

void desktop_draw(void) {
    if (!graphics_ready()) {
        desktop_draw_vga();
        return;
    }
    const struct framebuffer *fb = graphics_framebuffer();
    const struct input_state *in = input_state();
    const uint32_t bg = 0x001B2430, panel = 0x00232F3D, blue = 0x003B82F6;
    graphics_clear(bg);

    uint32_t panel_h = fb->height >= 48 ? 48 : fb->height;
    fill_rect(0, fb->height - panel_h, fb->width, panel_h, panel);
    fill_rect(8, fb->height - panel_h + 7, 112, panel_h - 14, blue);
    draw_text(20, fb->height - panel_h + 16, "START", 0x00FFFFFF, 2);

    if (launcher_open) {
        uint32_t menu_w = 220, menu_h = 150;
        uint32_t mx = 8, my = fb->height - panel_h - menu_h - 8;
        fill_rect(mx, my, menu_w, menu_h, 0x00E7EDF2);
        draw_border(mx, my, menu_w, menu_h, 0x0060A5FA);
        draw_text(mx + 14, my + 14, "APPLICATIONS", 0x00131A21, 1);
        fill_rect(mx + 10, my + 42, menu_w - 20, 38, 0x003B82F6);
        draw_text(mx + 22, my + 54, "TERMINAL", 0x00FFFFFF, 2);
        draw_text(mx + 22, my + 100, "DESKTOP", 0x00131A21, 2);
    }

    draw_window(compositor_window((uint32_t)starter_window), "DESKTOP", blue, 0x00E7EDF2, 0x00131A21);
    if (terminal_window) draw_window(compositor_window((uint32_t)terminal_window), "TERMINAL", 0x001F6FEB, 0x00131820, 0x00FFFFFF);
    draw_cursor(in->mouse_x, in->mouse_y, 0x00FFFFFF);
}

void desktop_update(void) {
    if (!graphics_ready()) {
        input_poll();
        previous_buttons = input_state()->buttons;
        desktop_draw_vga();
        return;
    }
    input_poll();
    const struct input_state *in = input_state();
    const struct framebuffer *fb = graphics_framebuffer();
    uint8_t clicked = (in->buttons & 1) && !(previous_buttons & 1);

    uint32_t panel_h = fb->height >= 48 ? 48 : fb->height;
    if (clicked && inside(in->mouse_x, in->mouse_y, 8, fb->height - panel_h + 7, 112, panel_h - 14))
        launcher_open = !launcher_open;

    if (launcher_open && clicked) {
        uint32_t my = fb->height - panel_h - 150 - 8;
        if (inside(in->mouse_x, in->mouse_y, 18, my + 42, 200, 38)) {
            if (!terminal_window) terminal_window = compositor_create_window(120, 90, 420, 260);
            launcher_open = 0;
        }
    }

    const struct window *w = compositor_window((uint32_t)starter_window);
    if (w && clicked && inside(in->mouse_x, in->mouse_y, w->x, w->y, w->width, 30)) {
        dragging = 1;
        drag_dx = in->mouse_x - (int32_t)w->x;
        drag_dy = in->mouse_y - (int32_t)w->y;
    }
    if (!(in->buttons & 1)) dragging = 0;
    if (dragging) {
        int32_t nx = in->mouse_x - drag_dx, ny = in->mouse_y - drag_dy;
        if (nx < 0) nx = 0; if (ny < 0) ny = 0;
        if ((uint32_t)nx + w->width > fb->width) nx = (int32_t)(fb->width - w->width);
        if ((uint32_t)ny + w->height > fb->height - panel_h) ny = (int32_t)(fb->height - panel_h - w->height);
        compositor_move_window((uint32_t)starter_window, (uint32_t)nx, (uint32_t)ny);
    }

    previous_buttons = in->buttons;
    desktop_draw();
}

void desktop_init(void) {
    compositor_init();
    const struct framebuffer *fb = graphics_framebuffer();
    launcher_open = 0; terminal_window = 0; dragging = 0; previous_buttons = 0;

    if (!graphics_ready()) {
        starter_window = 0;
        input_init(VGA_WIDTH, VGA_HEIGHT);
        desktop_draw_vga();
        return;
    }

    uint32_t ww = fb->width >= 500 ? 420 : fb->width > 220 ? fb->width - 30 : fb->width;
    uint32_t wh = fb->height >= 300 ? 240 : fb->height > 100 ? fb->height - 70 : fb->height;
    starter_window = compositor_create_window((fb->width - ww) / 2, (fb->height - 48 - wh) / 2, ww, wh);
    input_init(fb->width, fb->height);
    desktop_draw();
}
