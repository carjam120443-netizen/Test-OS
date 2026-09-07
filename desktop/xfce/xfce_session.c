#include <stdint.h>

/*
 * Test-OS XFCE session integration stub.
 *
 * This is deliberately freestanding: it defines the interface that the
 * future desktop/session manager will use once ring-3 execution, ELF loading,
 * graphics, IPC, and a POSIX-compatible libc are available.
 */

#define XFCE_SESSION_VERSION 1

struct xfce_session {
    uint32_t version;
    uint32_t display_ready;
    uint32_t input_ready;
    uint32_t ipc_ready;
};

static struct xfce_session session = {
    XFCE_SESSION_VERSION,
    0,
    0,
    0
};

void xfce_session_init(void) {
    session.version = XFCE_SESSION_VERSION;
    session.display_ready = 0;
    session.input_ready = 0;
    session.ipc_ready = 0;
}

int xfce_session_ready(void) {
    return session.display_ready && session.input_ready && session.ipc_ready;
}

uint32_t xfce_session_version(void) {
    return session.version;
}

void xfce_session_set_display_ready(int ready) {
    session.display_ready = ready ? 1u : 0u;
}

void xfce_session_set_input_ready(int ready) {
    session.input_ready = ready ? 1u : 0u;
}

void xfce_session_set_ipc_ready(int ready) {
    session.ipc_ready = ready ? 1u : 0u;
}
