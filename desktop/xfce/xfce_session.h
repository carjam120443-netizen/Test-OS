#ifndef TEST_OS_XFCE_SESSION_H
#define TEST_OS_XFCE_SESSION_H

#include <stdint.h>

void xfce_session_init(void);
int xfce_session_ready(void);
uint32_t xfce_session_version(void);
void xfce_session_set_display_ready(int ready);
void xfce_session_set_input_ready(int ready);
void xfce_session_set_ipc_ready(int ready);

#endif
