#ifndef TEST_OS_INPUT_H
#define TEST_OS_INPUT_H

#include <stdint.h>

struct input_state {
    int32_t mouse_x;
    int32_t mouse_y;
    uint8_t buttons;
    uint8_t key;
    uint8_t key_pressed;
};

void input_init(uint32_t width, uint32_t height);
void input_poll(void);
const struct input_state *input_state(void);

#endif
