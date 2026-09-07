#include "syscall.h"

__attribute__((section(".user.text")))
void user_init(void) {
    sys_write("Test-OS userspace: init is running in ring 3.\n");
    sys_write("XFCE userspace stack is now being built.\n");
    for (;;) sys_yield();
}
