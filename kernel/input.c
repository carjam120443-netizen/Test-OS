#include "input.h"

#define PS2_STATUS 0x64
#define PS2_DATA   0x60
#define PS2_CMD    0x64

static struct input_state state;
static uint32_t screen_width = 640;
static uint32_t screen_height = 480;
static uint8_t mouse_cycle;
static int8_t mouse_packet[3];
static uint8_t keyboard_extended;

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static int wait_input_clear(void) {
    for (uint32_t i = 0; i < 100000; ++i)
        if (!(inb(PS2_STATUS) & 2)) return 1;
    return 0;
}

static int wait_output_ready(void) {
    for (uint32_t i = 0; i < 100000; ++i)
        if (inb(PS2_STATUS) & 1) return 1;
    return 0;
}

static void mouse_write(uint8_t value) {
    if (!wait_input_clear()) return;
    outb(PS2_CMD, 0xD4);
    if (!wait_input_clear()) return;
    outb(PS2_DATA, value);
    if (wait_output_ready()) (void)inb(PS2_DATA);
}

static void keyboard_poll(void) {
    while (inb(PS2_STATUS) & 1) {
        uint8_t status = inb(PS2_STATUS);
        if (status & 0x20) break;
        uint8_t scancode = inb(PS2_DATA);
        if (scancode == 0xE0) { keyboard_extended = 1; continue; }
        if (scancode & 0x80) { keyboard_extended = 0; continue; }
        state.key_pressed = scancode;
        keyboard_extended = 0;
    }
}

static void mouse_poll(void) {
    while ((inb(PS2_STATUS) & 1) && (inb(PS2_STATUS) & 0x20)) {
        uint8_t value = inb(PS2_DATA);
        if (mouse_cycle == 0 && !(value & 0x08)) continue;
        mouse_packet[mouse_cycle++] = (int8_t)value;
        if (mouse_cycle < 3) continue;

        uint8_t flags = (uint8_t)mouse_packet[0];
        int32_t dx = mouse_packet[1];
        int32_t dy = mouse_packet[2];
        if (flags & 0x40) dx = 0;
        if (flags & 0x80) dy = 0;
        state.mouse_x += dx;
        state.mouse_y -= dy;
        if (state.mouse_x < 0) state.mouse_x = 0;
        if (state.mouse_y < 0) state.mouse_y = 0;
        if ((uint32_t)state.mouse_x >= screen_width) state.mouse_x = (int32_t)screen_width - 1;
        if ((uint32_t)state.mouse_y >= screen_height) state.mouse_y = (int32_t)screen_height - 1;
        state.buttons = flags & 7;
        mouse_cycle = 0;
    }
}

void input_init(uint32_t width, uint32_t height) {
    screen_width = width ? width : 640;
    screen_height = height ? height : 480;
    state.mouse_x = (int32_t)(screen_width / 2);
    state.mouse_y = (int32_t)(screen_height / 2);
    state.buttons = 0;
    state.key = 0;
    state.key_pressed = 0;
    mouse_cycle = 0;

    /* Enable the auxiliary PS/2 mouse and its data reporting. */
    if (wait_input_clear()) outb(PS2_CMD, 0xA8);
    mouse_write(0xF6);
    mouse_write(0xF4);
}

void input_poll(void) {
    state.key_pressed = 0;
    keyboard_poll();
    mouse_poll();
}

const struct input_state *input_state(void) { return &state; }
