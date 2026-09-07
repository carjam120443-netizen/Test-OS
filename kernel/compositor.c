#include "compositor.h"

static struct window windows[TESTOS_MAX_WINDOWS];
static uint32_t count;

void compositor_init(void) {
    count = 0;
    for (uint32_t i = 0; i < TESTOS_MAX_WINDOWS; ++i) windows[i].id = 0;
}

int compositor_create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (count >= TESTOS_MAX_WINDOWS || !width || !height) return -1;
    struct window *w = &windows[count];
    w->id = count + 1; w->x = x; w->y = y; w->width = width; w->height = height; w->flags = 1;
    return (int)w->id++ - 1;
}

const struct window *compositor_window(uint32_t id) {
    if (!id || id > count) return 0;
    return &windows[id - 1];
}
uint32_t compositor_window_count(void) { return count; }
