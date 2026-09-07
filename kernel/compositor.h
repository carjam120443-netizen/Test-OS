#ifndef TEST_OS_COMPOSITOR_H
#define TEST_OS_COMPOSITOR_H

#include <stdint.h>

#define TESTOS_MAX_WINDOWS 16
struct window { uint32_t id, x, y, width, height, flags; };

void compositor_init(void);
int compositor_create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
const struct window *compositor_window(uint32_t id);
uint32_t compositor_window_count(void);

#endif
